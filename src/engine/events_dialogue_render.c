#include "event_engine.h"
#include <string.h>

static JxFarPointer dialogue_image(JxEventContext *c,uint16_t displacement) {
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+displacement;
    JxFarPointer p={(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+(sum>>16)*0x1000)};
    return p;
}
static void dialogue_rectangle(JxEventContext *c,uint16_t operation,uint16_t x,
    uint16_t y,uint16_t width,uint16_t height,uint16_t displacement) {
    JxFarPointer image=dialogue_image(c,displacement);
    (void)JX_CALL(c,0x1832,operation,x,y,width,height,image.offset,image.segment);
}
static uint16_t dialogue_next_page(JxEventContext *c) {
    return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2);
}

/* 2F28:5E6C. Four 30-byte lines per page, revealed two bytes at a time.
 * Input and drawing of local text use typed native callbacks. Other video,
 * portrait, music and wait services keep their original address contracts.
 */
typedef JxGlyphFlow (*CheckedTextDraw)(void *,uint16_t,uint16_t,const uint8_t *);
static JxDialogueFlow render(JxEventContext *c,uint16_t portrait,JxFarPointer text,
    int16_t mode,JxInputPoll poll,CheckedTextDraw draw,void *opaque,const JxStdioServices *stdio) {
    uint16_t x,y,portrait_x=0,portrait_y=0;
    uint16_t actor=(uint16_t)(0xd04e + jx_word(c,0x875a)*68u);
    if(portrait==jx_word(c,(uint16_t)(actor+12))) {
        x=0;y=112;portrait_x=260;portrait_y=122;
    } else if(portrait!=999) {x=60;y=0;}
    else {x=30;y=56;}

    int length=jx_signed_word(jx_string_length(c,text));
    int pages=length/120,remainder=length%120;
    if(remainder!=0) {
        pages++;
        JxFarPointer padding={0x0dde,c->data_segment};
        for(int i=0;i<120-remainder;i++) (void)jx_string_append(c,text,padding);
    }
    uint8_t lines[4][32]={{0}};
    if(portrait!=999) {
        dialogue_rectangle(c,0x1e88,portrait_x,portrait_y,60,70,0x5140);
        if(stdio) {
            if(jx_portrait_draw_dos(c,portrait_x,portrait_y,(int16_t)jx_signed_word(portrait),stdio)!=JX_WRITE_RETURNED)
                return JX_DIALOGUE_STACK_FAILURE;
        } else (void)JX_CALL(c,0x1f2a,0x752e,portrait_x,portrait_y,portrait);
    }
    for(int page=0;page<pages;page++) {
        for(unsigned row=0;row<4;row++) {
            for(unsigned column=0;column<30;column++) {
                lines[row][column]=jx_far_byte(c,text,(uint16_t)(page*120+row*30+column));
                lines[row][31]=0;
            }
        }
        int interval=mode==0?3:(jx_word(c,0x08c8)!=0?2:1);
        (void)JX_CALL(c,0x1832,0x1810,dialogue_next_page(c));
        (void)JX_CALL(c,0x1832,0x135e,dialogue_next_page(c),
            (uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
        if(page>0) dialogue_rectangle(c,0x1c6c,x,y,260,80,0);
        dialogue_rectangle(c,0x1e88,x,y,260,80,0);
        JxFarPointer background=dialogue_image(c,0),shaded=dialogue_image(c,0x7d00);
        (void)JX_CALL(c,0x2f28,0x6c7e,background.offset,background.segment,
            shaded.offset,shaded.segment,0x5140,0xb584,c->data_segment);
        dialogue_rectangle(c,0x1c6c,x,y,260,80,0x7d00);
        if(mode<2) {
            (void)JX_CALL(c,0x2f28,0xa616,x,y,(uint16_t)(x+259),y,80);
            (void)JX_CALL(c,0x2f28,0xa616,x,(uint16_t)(y+79),(uint16_t)(x+259),(uint16_t)(y+79),80);
            (void)JX_CALL(c,0x2f28,0xa616,x,y,x,(uint16_t)(y+79),80);
            (void)JX_CALL(c,0x2f28,0xa616,(uint16_t)(x+259),y,(uint16_t)(x+259),(uint16_t)(y+79),80);
        }
        (void)JX_CALL(c,0x1832,0x1836,dialogue_next_page(c));
        jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
        jx_input_clear(c);
        unsigned position=0;
        uint16_t ticks=0;
        while(position<120) {
            JxInputEvent event;
            if(poll(opaque,&event)!=0) continue;
            if(event.type==8) {
                ticks++;
                if(jx_signed_word(ticks)%interval!=0) continue;
                /* Consecutive blank pairs are skipped on the same tick,
                 * but music is polled again for every skipped pair. */
                while(position<120) {
                    (void)JX_CALL0(c,0x2f28,0x856c);
                    unsigned row=position/30,column=position%30;
                    uint8_t pair[3]={lines[row][column],lines[row][column+1],0};
                    position+=2;
                    if(pair[0]==' ' && pair[1]==' ') continue;
                    if(draw(opaque,(uint16_t)(x+10+column*8),(uint16_t)(y+2+row*20),pair)==JX_GLYPH_STALLED)
                        return JX_DIALOGUE_GLYPH_STALLED;
                    break;
                }
            } else if(event.type==0 && (event.value1==0x39 || event.value1==0x1c) && mode==0) {
                for(unsigned row=0;row<4;row++)
                    if(draw(opaque,(uint16_t)(x+10),(uint16_t)(y+2+row*20),lines[row])==JX_GLYPH_STALLED)
                        return JX_DIALOGUE_GLYPH_STALLED;
                break;
            }
        }
        if(jx_word(c,0x08c8)!=0) {
            uint16_t duration=(uint16_t)(mode>2?(mode-2)*150+18:18);
            (void)JX_CALL(c,0x1f2a,0x2dbc,duration);
        } else (void)JX_CALL0(c,0x1f2a,0x731a);
        memset(lines,0,sizeof(lines));
    }
    dialogue_rectangle(c,0x1c6c,x,y,260,80,0);
    if(portrait!=999) dialogue_rectangle(c,0x1c6c,portrait_x,portrait_y,60,70,0x5140);
    return JX_DIALOGUE_RETURNED;
}

typedef struct {JxInputPoll poll;JxTextDraw draw;void *opaque;} LegacyText;
static int legacy_poll(void *opaque,JxInputEvent *event) {
    LegacyText *host=opaque;return host->poll(host->opaque,event);
}
static JxGlyphFlow legacy_draw(void *opaque,uint16_t x,uint16_t y,const uint8_t *text) {
    LegacyText *host=opaque;host->draw(host->opaque,x,y,text);return JX_GLYPH_RETURNED;
}
void jx_dialogue_render(JxEventContext *c,uint16_t portrait,JxFarPointer text,
    int16_t mode,JxInputPoll poll,JxTextDraw draw,void *opaque) {
    LegacyText host={poll,draw,opaque};
    (void)render(c,portrait,text,mode,legacy_poll,legacy_draw,&host,NULL);
}

typedef struct {JxEventContext *context;JxFarPointer text;JxInputPoll poll;void *opaque;} FontText;
static int font_poll(void *opaque,JxInputEvent *event) {
    FontText *host=opaque;return host->poll(host->opaque,event);
}
static JxGlyphFlow font_draw(void *opaque,uint16_t x,uint16_t y,const uint8_t *text) {
    FontText *host=opaque;
    unsigned index=0;
    do {jx_far_set_byte(host->context,host->text,(uint16_t)index,text[index]);} while(text[index++]!=0);
    return jx_font_draw_text(host->context,x,y,host->text).flow;
}
/* The caller owns a private 32-byte far scratch region for local glyph text,
 * separate from the 1024-byte dialogue workspace and all live game data. */
JxGlyphFlow jx_dialogue_render_font(JxEventContext *c,uint16_t portrait,JxFarPointer text,
    int16_t mode,JxInputPoll poll,void *opaque,JxFarPointer glyph_text) {
    FontText host={c,glyph_text,poll,opaque};
    return (JxGlyphFlow)render(c,portrait,text,mode,font_poll,font_draw,&host,NULL);
}

JxDialogueFlow jx_dialogue_render_stdio(JxEventContext *c,uint16_t portrait,JxFarPointer text,
    int16_t mode,JxInputPoll poll,void *opaque,JxFarPointer glyph_text,const JxStdioServices *stdio) {
    FontText host={c,glyph_text,poll,opaque};
    return render(c,portrait,text,mode,font_poll,font_draw,&host,stdio);
}
