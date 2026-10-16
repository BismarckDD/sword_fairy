#include "event_engine.h"

/* 1000:3292. abs(-32768) remains 0x8000 in the original 16-bit ABI. */
uint16_t jx_absolute_word(uint16_t value) {
    return value&0x8000?(uint16_t)(0u-value):value;
}
static void put_far_word(JxEventContext *c,JxFarPointer p,uint16_t value) {
    uint32_t address=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[address]=(uint8_t)value;
    c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 2F28:A5A2. Capture both words before ordered writes; overlaps allowed.
 * Word access at offset FFFF uses the next physical byte, not offset 0. */
void jx_swap_words(JxEventContext *c,JxFarPointer first,JxFarPointer second) {
    uint16_t a=jx_far_word(c,first.segment,first.offset);
    uint16_t b=jx_far_word(c,second.segment,second.offset);
    put_far_word(c,first,b);put_far_word(c,second,a);
}
/* 2F28:A5CA. Inclusive signed endpoints, with wrapping word counter. */
void jx_draw_vertical(JxEventContext *c,uint16_t x,uint16_t y1,uint16_t y2,uint16_t color) {
    if(jx_signed_word(y1)>jx_signed_word(y2)) {uint16_t t=y1;y1=y2;y2=t;}
    for(uint16_t y=y1;jx_signed_word(y)<=jx_signed_word(y2);y=(uint16_t)(y+1))
        (void)JX_CALL(c,0x1832,0x14ce,x,y,color);
}
/* 2F28:A616. Original x-major recurrence, including steep-line behavior.
 * Vertical helper call does NOT return early: start pixel is drawn again. */
void jx_draw_line(JxEventContext *c,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color) {
    if(jx_signed_word(x1)>jx_signed_word(x2)) {
        uint16_t t=x1;x1=x2;x2=t;t=y1;y1=y2;y2=t;
    }
    if(x1==x2) jx_draw_vertical(c,x1,y1,y2,color);
    uint16_t step=jx_signed_word(y2)>jx_signed_word(y1)?1:0xffff;
    uint16_t dx=(uint16_t)(x2-x1),dy=jx_absolute_word((uint16_t)(y2-y1));
    uint16_t error=(uint16_t)(2u*dy-dx);
    uint16_t diagonal=(uint16_t)(2u*(uint16_t)(dy-dx)),straight=(uint16_t)(2u*dy),y=y1;
    (void)JX_CALL(c,0x1832,0x14ce,x1,y,color);
    for(uint16_t x=(uint16_t)(x1+1);jx_signed_word(x)<=jx_signed_word(x2);x=(uint16_t)(x+1)) {
        if(jx_signed_word(error)>=0) {y=(uint16_t)(y+step);error=(uint16_t)(error+diagonal);}
        else error=(uint16_t)(error+straight);
        (void)JX_CALL(c,0x1832,0x14ce,x,y,color);
    }
}
