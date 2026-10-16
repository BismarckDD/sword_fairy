#include "event_engine.h"

static void reveal_pixel(JxEventContext *c,JxFarPointer image,uint16_t x,uint16_t y) {
    uint32_t address=((uint32_t)image.segment*16+image.offset+y*320u+x)&0xfffff;
    /* The original CBW sign-extends palette indices 128..255 at the ABI. */
    uint16_t color=(uint16_t)jx_signed_byte(c->memory[address]);
    (void)JX_CALL(c,0x1832,0x14ce,x,y,color);
}

/* Overlay 009:0754. Modes 0/1 scan top-down/bottom-up. Every other value
 * alternates opposite rows, retaining the original's second visits. */
void jx_transition_rows(JxEventContext *c,JxFarPointer image,uint16_t mode) {
    for(uint16_t row=0;row<200;row++) {
        for(uint16_t x=0;x<320;x++) {
            if(mode!=1) reveal_pixel(c,image,x,row);
            if(mode!=0) reveal_pixel(c,image,x,(uint16_t)(199-row));
        }
    }
}

/* Overlay 009:09AC. The analogous column-major reveal. */
void jx_transition_columns(JxEventContext *c,JxFarPointer image,uint16_t mode) {
    for(uint16_t column=0;column<320;column++) {
        for(uint16_t y=0;y<200;y++) {
            if(mode!=1) reveal_pixel(c,image,column,y);
            if(mode!=0) reveal_pixel(c,image,(uint16_t)(319-column),y);
        }
    }
}

static void reveal_border(JxEventContext *c,JxFarPointer image,uint16_t inset) {
    for(uint16_t x=inset;x<320-inset;x++) {
        reveal_pixel(c,image,x,inset);
        reveal_pixel(c,image,x,(uint16_t)(199-inset));
    }
    for(uint16_t y=inset;y<200-inset;y++) {
        reveal_pixel(c,image,inset,y);
        reveal_pixel(c,image,(uint16_t)(319-inset),y);
    }
}

/* Overlay 009:0C60/0F0A. All 100 rectangular borders, in opposite orders.
 * Corners are intentionally visited twice, matching the original calls. */
void jx_transition_inward(JxEventContext *c,JxFarPointer image) {
    for(uint16_t inset=0;inset<100;inset++) reveal_border(c,image,inset);
}
void jx_transition_outward(JxEventContext *c,JxFarPointer image) {
    for(int inset=99;inset>=0;inset--) reveal_border(c,image,(uint16_t)inset);
}

/* 2F28:753A. Four interleaved inward border passes, clockwise order.
 * Unlike overlay009, palette bytes are zero-extended in this entry. */
static void reveal_unsigned(JxEventContext *c,JxFarPointer image,uint16_t x,uint16_t y) {
    uint32_t at=((uint32_t)image.segment*16+image.offset+y*320u+x)&0xfffff;
    (void)JX_CALL(c,0x1832,0x14ce,x,y,c->memory[at]);
}
void jx_transition_border_passes(JxEventContext *c,JxFarPointer image) {
    for(uint16_t phase=0;phase<4;phase++) {
        for(uint16_t inset=phase;inset<100;inset=(uint16_t)(inset+4)) {
            for(uint16_t i=0;i<320-2*inset;i++)reveal_unsigned(c,image,(uint16_t)(inset+i),inset);
            for(uint16_t i=0;i<200-2*inset;i++)reveal_unsigned(c,image,(uint16_t)(319-inset),(uint16_t)(inset+i));
            for(uint16_t i=0;i<320-2*inset;i++)reveal_unsigned(c,image,(uint16_t)(319-inset-i),(uint16_t)(199-inset));
            for(uint16_t i=0;i<200-2*inset;i++)reveal_unsigned(c,image,inset,(uint16_t)(199-inset-i));
        }
        (void)JX_CALL(c,0x1f2a,0x720e,300);
    }
}
