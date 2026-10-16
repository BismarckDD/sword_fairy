#include "event_engine.h"
/* 1F2A:8CA8. Quantity is deliberately not part of this predicate. */
uint16_t jx_inventory_has(JxEventContext *c,uint16_t item) {
    for(unsigned slot=0;slot<64;slot++) if(jx_word(c,(uint16_t)(0x878a+slot*4))==item) return 1;
    return 0;
}
/* 1F2A:8BD6. Stable filter; preserve duplicates, clear unused slots. */
void jx_inventory_compact(JxEventContext *c) {
    uint16_t items[64][2]={{0}};unsigned used=0;
    for(unsigned slot=0;slot<64;slot++) {
        uint16_t at=(uint16_t)(0x878a+slot*4);
        uint16_t item=jx_word(c,at),count=jx_word(c,(uint16_t)(at+2));
        if(jx_signed_word(item)>0 && jx_signed_word(count)>0) {
            items[used][0]=item;items[used][1]=count;used++;
        }
    }
    for(unsigned slot=0;slot<64;slot++) {
        jx_set_word(c,(uint16_t)(0x878a+slot*4),items[slot][0]);
        jx_set_word(c,(uint16_t)(0x878c+slot*4),items[slot][1]);
    }
}
/* 1F2A:8F28. Subtract from first match with word wrap, then compact all.
 * No match means no compaction, even if other slots are malformed. */
void jx_inventory_remove(JxEventContext *c,uint16_t item,uint16_t amount) {
    for(unsigned slot=0;slot<64;slot++) {
        uint16_t at=(uint16_t)(0x878a+slot*4);
        if(jx_word(c,at)==item) {
            jx_set_word(c,(uint16_t)(at+2),(uint16_t)(jx_word(c,(uint16_t)(at+2))-amount));
            jx_inventory_compact(c);return;
        }
    }
}
static uint16_t skills_offset(uint16_t actor) {return (uint16_t)(0xd082u+actor*68u);}
/* 1F2A:8E1C. Read fixed-DS signed bytes, but original final huge memcpy
 * carries its segment across a 64-KiB destination boundary. */
void jx_actor_skills_compact(JxEventContext *c,uint16_t actor) {
    uint8_t skills[16];unsigned used=0;uint16_t start=skills_offset(actor);
    for(unsigned i=0;i<16;i++) skills[i]=255;
    for(unsigned i=0;i<16;i++) {
        uint8_t value=jx_ds(c)[(uint16_t)(start+i)];
        if(value<128) skills[used++]=value;
    }
    uint32_t destination=(uint32_t)c->data_segment*16+start;
    for(unsigned i=0;i<16;i++) c->memory[(destination+i)&0xfffff]=skills[i];
}
/* 1F2A:8EC4. Full signed-word ID compared against sign-extended byte. */
void jx_actor_skill_remove(JxEventContext *c,int16_t skill,uint16_t actor) {
    uint16_t start=skills_offset(actor);
    for(unsigned i=0;i<16;i++) {
        uint16_t at=(uint16_t)(start+i);
        if(jx_signed_byte(jx_ds(c)[at])==skill) {
            jx_ds(c)[at]=255;jx_actor_skills_compact(c,actor);return;
        }
    }
}

/* 1F2A:8CEA, including the notification omitted by grant_item_core.
 * Number scratch starts 30 bytes into the original local text area.
 * sprintf remains an explicit CRT dependency with its original arguments. */
static JxGlyphFlow inventory_grant_impl(JxEventContext *c,int16_t item,int16_t amount,
    JxFarPointer workspace,JxInputPoll poll,void *opaque,const JxFarPointer *format_workspace) {
    if(item<0 || amount<=0) return JX_GLYPH_RETURNED;
    JxFarPointer text=workspace,number={(uint16_t)(workspace.offset+30),workspace.segment};
    (void)jx_string_copy(c,text,(JxFarPointer){0x05c4,c->data_segment});
    (void)jx_string_append(c,text,(JxFarPointer){(uint16_t)(0x8022u+(uint16_t)item*28u),c->data_segment});
    if(format_workspace) {
        JxFarPointer argument={(uint16_t)(format_workspace->offset+532),format_workspace->segment};
        jx_far_set_byte(c,argument,0,(uint8_t)amount);
        jx_far_set_byte(c,argument,1,(uint8_t)((uint16_t)amount>>8));
        (void)jx_sprintf_format(c,number,(JxFarPointer){0x05c9,c->data_segment},argument,*format_workspace);
    } else {
        (void)JX_CALL(c,0x1000,0x2a8c,number.offset,number.segment,0x05c9,c->data_segment,(uint16_t)amount);
    }
    (void)jx_string_append(c,text,number);
    (void)jx_string_append(c,text,(JxFarPointer){0x05cd,c->data_segment});
    (void)JX_CALL(c,0x1832,0x1e88,100,100,160,17,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    JxTextResult result=jx_font_draw_text(c,100,100,text);
    if(result.flow==JX_GLYPH_STALLED) return result.flow;
    jx_wait_key(c,poll,opaque);
    (void)JX_CALL(c,0x1832,0x1c6c,100,100,160,17,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    for(unsigned slot=0;slot<64;slot++) {
        uint16_t at=(uint16_t)(0x878a+slot*4);
        if(jx_word(c,at)==(uint16_t)item) {
            jx_set_word(c,(uint16_t)(at+2),(uint16_t)(jx_word(c,(uint16_t)(at+2))+(uint16_t)amount));
            return JX_GLYPH_RETURNED;
        }
    }
    jx_set_word(c,0x8886,(uint16_t)item);jx_set_word(c,0x8888,(uint16_t)amount);
    jx_inventory_compact(c);return JX_GLYPH_RETURNED;
}

JxGlyphFlow jx_inventory_grant(JxEventContext *c,int16_t item,int16_t amount,
    JxFarPointer workspace,JxInputPoll poll,void *opaque) {
    return inventory_grant_impl(c,item,amount,workspace,poll,opaque,NULL);
}
JxGlyphFlow jx_inventory_grant_formatted(JxEventContext *c,int16_t item,int16_t amount,
    JxFarPointer workspace,JxFarPointer format_workspace,JxInputPoll poll,void *opaque) {
    return inventory_grant_impl(c,item,amount,workspace,poll,opaque,&format_workspace);
}
