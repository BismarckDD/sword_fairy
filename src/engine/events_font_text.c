#include "event_engine.h"

/* 1F2A:06F2/0718. Colors occupy words but accept only a low byte. */
void jx_font_set_colors(JxEventContext *c,uint16_t foreground,uint16_t background,uint16_t shadow) {
    jx_set_word(c,0x0090,(uint16_t)(foreground&255));
    jx_set_word(c,0x0094,(uint16_t)(background&255));
    jx_set_word(c,0x0092,(uint16_t)(shadow&255));
}
void jx_font_set_mode(JxEventContext *c,uint16_t mode) {jx_set_word(c,0x008e,mode);}

static JxGlyphFlow glyph(JxEventContext *c,int wide,uint16_t x,uint16_t y,
    uint16_t color,uint16_t mode) {
    uint16_t background=jx_word(c,0x0094);
    return wide?jx_font_raster_double(c,x,y,color,mode,background):
        jx_font_raster_single(c,x,y,color,mode,background);
}

/* 1F2A:0728. Draw mixed 8/16-pixel text, including the original ordered
 * shadow strokes. The connected implementation propagates a raster stall;
 * a host must never treat that flow as a normal original function return. */
static JxTextResult font_text(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer text,int timed,uint16_t delay) {
    uint16_t draw_x=(uint16_t)(x+jx_word(c,0x0086));
    uint16_t draw_y=(uint16_t)(y+jx_word(c,0x0088));
    uint16_t original_mode=jx_word(c,0x008e);
    uint16_t first_shadow=(original_mode&0x81)==0x81?1:0;
    uint16_t body_mode=first_shadow?(uint16_t)(original_mode&0xfffe):original_mode;
    if(jx_signed_word(x)<0 || jx_signed_word(x)>jx_signed_word(jx_word(c,0x008a)) ||
       jx_signed_word(y)<0 || jx_signed_word(y)>jx_signed_word(jx_word(c,0x008c)))
        return (JxTextResult){JX_GLYPH_RETURNED,-2};
    int length=jx_signed_word(jx_string_length(c,text));
    for(uint16_t index=0;jx_signed_word(index)<length;) {
        uint8_t first=jx_far_byte(c,text,index);
        int wide=first>127;
        if(wide) {
            uint16_t code=(uint16_t)((first<<8)|jx_far_byte(c,text,(uint16_t)(index+1)));
            (void)jx_font_load_double(c,code);
        } else (void)jx_font_load_single(c,first);

        /* Mode is reread after loading, but the main mode and first-shadow
         * flag were captured at entry. Colors are reread for every stroke. */
        uint16_t style=(uint16_t)(jx_word(c,0x008e)&0xf0);
        static const int8_t strokes[5][8][2]={
            {{0,-1},{-1,-1},{-1,0}},
            {{-1,0},{-1,1},{0,1}},
            {{1,0},{1,-1},{0,-1}},
            {{1,0},{1,1},{0,1}},
            {{-1,-1},{-1,1},{1,-1},{1,1},{-1,0},{1,0},{0,-1},{0,1}}
        };
        if(style>=0x80 && style<=0xc0) {
            unsigned variant=(style-0x80)/16,amount=variant==4?8:3;
            for(unsigned stroke=0;stroke<amount;stroke++) {
                unsigned selected=stroke;
                /* ASCII's 0x90 style reverses its first two strokes. */
                if(!wide && variant==1 && stroke<2) selected=1-stroke;
                uint16_t sx=(uint16_t)(draw_x+strokes[variant][selected][0]);
                uint16_t sy=(uint16_t)(draw_y+strokes[variant][selected][1]);
                if(glyph(c,wide,sx,sy,jx_word(c,0x0092),stroke==0?first_shadow:0)==JX_GLYPH_STALLED)
                    return (JxTextResult){JX_GLYPH_STALLED,0};
            }
        }
        if(glyph(c,wide,draw_x,draw_y,jx_word(c,0x0090),body_mode)==JX_GLYPH_STALLED)
            return (JxTextResult){JX_GLYPH_STALLED,0};
        draw_x=(uint16_t)(draw_x+(wide?16:8));
        index=(uint16_t)(index+(wide?2:1));
        if(timed && wide)(void)JX_CALL(c,0x1f2a,0x2dbc,delay);
    }
    return (JxTextResult){JX_GLYPH_RETURNED,0};
}

JxTextResult jx_font_draw_text(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer text){return font_text(c,x,y,text,0,0);}
/* 1F2A:0E04. Only double-byte glyphs incur the original timer delay. */
JxTextResult jx_font_draw_text_timed(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer text,uint16_t delay){return font_text(c,x,y,text,1,delay);}
