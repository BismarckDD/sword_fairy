#include "event_engine.h"

/* Compiler huge-pointer addition preserves the low 16-bit offset and adds
 * 0x1000 to the segment on each 64-KiB carry. Keep that representation at the
 * graphics ABI, since physically equal far pointers need not have equal words. */
static void scanline(JxEventContext *c,uint16_t image_off,uint16_t image_seg,
    unsigned source_index,uint16_t x,uint16_t y,uint16_t width) {
    uint32_t sum=(uint32_t)image_off+source_index;
    uint16_t offset=(uint16_t)sum;
    uint16_t segment=(uint16_t)(image_seg+(sum>>16)*4096u);
    (void)JX_CALL(c,0x1832,0x1c6c,x,y,width,1,offset,segment);
}

/* Overlay 009:0000. Reveal even scanlines from the two edges and odd
 * scanlines from the center, expanding one pixel per pass. */
void jx_transition_interlaced(JxEventContext *c,uint16_t image_off,uint16_t image_seg) {
    for(uint16_t half_width=1;half_width<=160;half_width++) {
        for(uint16_t pair=0;pair<100;pair++) {
            unsigned even=pair*640u;
            scanline(c,image_off,image_seg,even,0,(uint16_t)(pair*2),half_width);
            scanline(c,image_off,image_seg,even+320-half_width,(uint16_t)(320-half_width),(uint16_t)(pair*2),half_width);
            scanline(c,image_off,image_seg,even+480-half_width,(uint16_t)(160-half_width),(uint16_t)(pair*2+1),(uint16_t)(half_width*2));
        }
    }
}

/* Overlay 009:01A4. Each successive row starts one pass later. */
void jx_transition_cascade(JxEventContext *c,uint16_t image_off,uint16_t image_seg) {
    for(int pass=1;pass<=360;pass++) {
        for(uint16_t row=0;row<200;row++) {
            int width=2*(pass-row);
            if(width>0 && width<=320) {
                uint16_t left=(uint16_t)((320-width)/2);
                scanline(c,image_off,image_seg,row*320u+left,left,row,(uint16_t)width);
            }
        }
    }
}
