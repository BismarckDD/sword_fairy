#include "event_engine.h"

/* 1F2A:04A0 and 05BA differ only in width (8/16). Glyph bits begin at
 * DS:2724; DS:0046 holds the far pixel workspace. Bit reads are repeated
 * per pixel to preserve overlapping glyph/pixel-buffer behavior. */
static JxGlyphFlow raster(JxEventContext *c,unsigned width,uint16_t x,uint16_t y,
    uint16_t foreground,uint16_t mode,uint16_t background) {
    JxFarPointer pixels={jx_word(c,0x0046),jx_word(c,0x0048)};
    (void)jx_memory_fill(c,pixels,background,(uint16_t)(width*16));
    pixels.offset=jx_word(c,0x0046);pixels.segment=jx_word(c,0x0048);
    uint16_t row_mode=mode;
    for(unsigned row=0;row<16;row++) {
        for(unsigned column=0;column<width;column++) {
            uint8_t bits=jx_ds(c)[0x2724+row*(width/8)+column/8];
            uint8_t color=(uint8_t)((bits&(0x80>>(column%8)))?foreground:background);
            jx_far_set_byte(c,pixels,(uint16_t)(row*width+column),color);
        }
        if(row==15) break;
        /* AND only affects the low BYTE of the original mode argument. */
        row_mode=(uint16_t)((row_mode&0xff00)|(row_mode&0x0e));
        if((row_mode&255)==0) continue;
        if(row_mode==8 || row_mode==12) foreground++;
        else if(row_mode==10) foreground--;
        else return JX_GLYPH_STALLED;
        /* The apparent second 0x0c case at 054F/0687 is unreachable:
         * all other values loop on the repeated comparison forever.
         * The first 0x0c branch increments on every row; its local flag
         * stays 1, so the misleading AL adjustment never executes. */
    }
    pixels.offset=jx_word(c,0x0046);pixels.segment=jx_word(c,0x0048);
    if(mode&1) (void)JX_CALL(c,0x1832,0x1c6c,x,y,(uint16_t)width,16,pixels.offset,pixels.segment);
    else (void)JX_CALL(c,0x1832,0x1a28,x,y,(uint16_t)width,16,pixels.offset,pixels.segment,background);
    return JX_GLYPH_RETURNED;
}
JxGlyphFlow jx_font_raster_single(JxEventContext *c,uint16_t x,uint16_t y,
    uint16_t foreground,uint16_t mode,uint16_t background) {
    return raster(c,8,x,y,foreground,mode,background);
}
JxGlyphFlow jx_font_raster_double(JxEventContext *c,uint16_t x,uint16_t y,
    uint16_t foreground,uint16_t mode,uint16_t background) {
    return raster(c,16,x,y,foreground,mode,background);
}
