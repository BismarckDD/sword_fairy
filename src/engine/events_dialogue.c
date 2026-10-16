#include "event_engine.h"

/* 1000:325F. The CRT uses a mutable classification table, not a hardcoded
 * ASCII range. Signed characters also index this original DS table. */
uint16_t jx_character_is_digit(JxEventContext *c,int16_t character) {
    return (uint16_t)(jx_ds(c)[(uint16_t)(0x15f7+character)]&4);
}

/* 2F28:5ACA. A two-byte speaker prefix precedes each text fragment.
 * a/b/c select party actors, d selects the supplied NPC, and decimal
 * prefixes select an NPC or an explicit portrait number. Return -2 means
 * end of text; 999 denotes text without a portrait. The two NPC arguments
 * differ in the original ABI: the first chooses d's name, the second its
 * portrait and whether portraits are enabled at all.
 *
 * The far output buffer is caller-owned. Preserve original wrapping and
 * alias effects; no guessed scratch address or fixed 1024-byte truncation.
 */
int jx_dialogue_fragment(JxEventContext *c,int16_t name_npc,JxFarPointer output,
    int16_t portrait_npc,JxFarPointer text) {
    uint16_t cursor=jx_word(c,0x2b5a);
    uint8_t first=jx_far_byte(c,text,cursor);
    uint8_t second=jx_far_byte(c,text,(uint16_t)(cursor+1));
    uint16_t portrait=999;
    int named=0;
    if(first=='a' || first=='b' || first=='c' || first=='d') {
        uint16_t name_address;
        if(first=='d') {
            portrait=jx_word(c,(uint16_t)(0x889c+(uint16_t)portrait_npc*86u+12));
            name_address=(uint16_t)(0x889c+(uint16_t)name_npc*86u);
        } else {
            uint16_t actor=jx_word(c,(uint16_t)(0x875a+2*(first-'a')));
            name_address=(uint16_t)(0xd04e + actor*68u);
            portrait=jx_word(c,(uint16_t)(name_address+12));
        }
        JxFarPointer name={name_address,c->data_segment};
        (void)jx_string_copy(c,output,name);
        named=1;
    } else if(jx_character_is_digit(c,(int16_t)jx_signed_byte(first)) &&
              jx_character_is_digit(c,(int16_t)jx_signed_byte(second))) {
        portrait=(uint16_t)((jx_signed_byte(first)-'0')*10+jx_signed_byte(second)-'0');
        if(jx_signed_word(portrait)<30) {
            JxFarPointer name={(uint16_t)(0x889c+portrait*86u),c->data_segment};
            (void)jx_string_copy(c,output,name);
            portrait=jx_word(c,(uint16_t)(name.offset+12));
        }
        if(name_npc>=0) named=1;
    }
    if(portrait_npc<0) portrait=999;
    if(first==0) return -2;

    uint16_t index=0;
    if(named) {
        for(;index<30;index++) {
            if(jx_far_byte(c,output,index)==0) {
                jx_far_set_byte(c,output,index,':');break;
            }
        }
        /* If no NUL was found, original code leaves byte 30 untouched. */
        for(index++;index<34;index++) jx_far_set_byte(c,output,index,' ');
    } else {
        for(;index<4;index++) jx_far_set_byte(c,output,index,' ');
    }
    uint16_t padding=(uint16_t)(named?32:2);
    for(index=2;;index++) {
        uint8_t ch=jx_far_byte(c,text,(uint16_t)(jx_word(c,0x2b5a)+index));
        if(ch==0 || (ch>33 && ch<128)) break;
        jx_far_set_byte(c,output,(uint16_t)(index+padding),ch);
    }
    jx_far_set_byte(c,output,(uint16_t)(index+padding),0);
    jx_set_word(c,0x2b5a,(uint16_t)(jx_word(c,0x2b5a)+index));
    return jx_signed_word(portrait);
}

/* 2F28:648E. The original allocates its text workspace on the DOS stack.
 * The host supplies equivalent private far storage explicitly; it must not
 * overlap live game data or text. Rendering may modify the workspace, just
 * as 5E6C does. This interface preserves those changes between fragments.
 * Original well-formed content fits the 1024-byte workspace; the original
 * parser does not check that bound. EMS and rendering remain host services.
 */
static JxDialogueFlow dialogue_run(JxEventContext *c,int16_t npc,JxFarPointer text,
    uint16_t mode,JxFarPointer workspace,JxInputPoll poll,JxTextDraw draw,void *opaque,
    const JxFarPointer *glyph_text,const JxStdioServices *stdio) {
    if(text.offset==0 && text.segment==0) return JX_DIALOGUE_RETURNED;
    jx_set_word(c,0x2b5a,0);
    for(;;) {
        for(uint16_t page=0;page<4;page++) {
            uint16_t status=(uint16_t)JX_CALL(c,0x1503,0x007d,page,
                (uint16_t)(175+page),jx_word(c,0x9332));
            if(status!=0) return JX_DIALOGUE_RETURNED;
        }
        int portrait=jx_dialogue_fragment(c,npc,workspace,npc,text);
        if(portrait < -1) return JX_DIALOGUE_RETURNED;
        if(stdio) {
            JxDialogueFlow flow=jx_dialogue_render_stdio(c,(uint16_t)portrait,workspace,
                (int16_t)jx_signed_word(mode),poll,opaque,*glyph_text,stdio);
            if(flow!=JX_DIALOGUE_RETURNED) return flow;
        } else if(glyph_text) {
            if(jx_dialogue_render_font(c,(uint16_t)portrait,workspace,
                (int16_t)jx_signed_word(mode),poll,opaque,*glyph_text)==JX_GLYPH_STALLED)
                return JX_DIALOGUE_GLYPH_STALLED;
        } else if(draw) jx_dialogue_render(c,(uint16_t)portrait,workspace,
            (int16_t)jx_signed_word(mode),poll,draw,opaque);
        else (void)JX_CALL(c,0x2f28,0x5e6c,(uint16_t)portrait,
            workspace.offset,workspace.segment,mode);
    }
}
void jx_dialogue_run(JxEventContext *c,int16_t npc,JxFarPointer text,
    uint16_t mode,JxFarPointer workspace) {
    (void)dialogue_run(c,npc,text,mode,workspace,NULL,NULL,NULL,NULL,NULL);
}
/* Connected high-level path: parsing and pagination both execute recovered
 * C; only the underlying services and native text/input callbacks remain. */
void jx_dialogue_show(JxEventContext *c,int16_t npc,JxFarPointer text,
    uint16_t mode,JxFarPointer workspace,JxInputPoll poll,JxTextDraw draw,void *opaque) {
    (void)dialogue_run(c,npc,text,mode,workspace,poll,draw,opaque,NULL,NULL);
}
JxGlyphFlow jx_dialogue_show_font(JxEventContext *c,int16_t npc,JxFarPointer text,
    uint16_t mode,JxFarPointer workspace,JxInputPoll poll,void *opaque,JxFarPointer glyph_text) {
    return (JxGlyphFlow)dialogue_run(c,npc,text,mode,workspace,poll,NULL,opaque,&glyph_text,NULL);
}

JxDialogueFlow jx_dialogue_show_stdio(JxEventContext *c,int16_t npc,JxFarPointer text,
    uint16_t mode,JxFarPointer workspace,JxInputPoll poll,void *opaque,JxFarPointer glyph_text,const JxStdioServices *stdio) {
    return dialogue_run(c,npc,text,mode,workspace,poll,NULL,opaque,&glyph_text,stdio);
}
