#include "event_engine.h"
#include <assert.h>

/* 2F28:A7FE. Valid DOS DAC channels are 0..63. Outside that domain the
 * original can return an uninitialized stack word if no distance improves
 * 20000; report -1 in that indeterminate case, rather than inventing an index.
 * Distance comparisons intentionally retain signed 16-bit wrap semantics. */
int jx_palette_nearest(JxEventContext *c,uint8_t red,uint8_t green,uint8_t blue,
    JxFarPointer palette,int16_t early_threshold) {
    int best_distance=20000,best=-1;
    for(int index=0;index<256;index++) {
        int dr=(int)jx_far_byte(c,palette,0)-red;
        int dg=(int)jx_far_byte(c,palette,1)-green;
        int db=(int)jx_far_byte(c,palette,2)-blue;
        int distance=jx_signed_word((uint16_t)(dr*dr+dg*dg+db*db));
        if(distance<early_threshold) return index;
        if(distance<best_distance) {best_distance=distance;best=index;}
        palette.offset=(uint16_t)(palette.offset+3);
    }
    return best;
}

/* Overlay 009:02B2. Nine palette-quantized blend frames, then a transparent
 * final blit. Unlike the two reveal effects, these pixel pointers wrap their
 * offset only, retaining the segment. Buffers may overlap. */
void jx_transition_blend(JxEventContext *c,uint16_t x,uint16_t y,uint16_t width,uint16_t height,
    JxFarPointer first,JxFarPointer second,JxFarPointer output,JxFarPointer palette) {
    uint16_t count=(uint16_t)((uint32_t)width*height);
    int16_t threshold=(int16_t)(count/256);
    for(unsigned weight=0;weight<=64;weight+=8) {
        for(uint16_t pixel=0;pixel<count;pixel++) {
            uint8_t a=jx_far_byte(c,first,pixel),b=jx_far_byte(c,second,pixel);
            if(a==b) {
                jx_far_set_byte(c,output,pixel,a);
                continue;
            }
            uint8_t color_a[3],color_b[3],mixed[3];
            for(uint16_t channel=0;channel<3;channel++)
                color_a[channel]=jx_far_byte(c,palette,(uint16_t)(3*a+channel));
            for(uint16_t channel=0;channel<3;channel++)
                color_b[channel]=b==255 ? color_a[channel] : jx_far_byte(c,palette,(uint16_t)(3*b+channel));
            for(unsigned channel=0;channel<3;channel++)
                mixed[channel]=(uint8_t)((color_a[channel]*(64-weight)+color_b[channel]*weight)>>6);
            int nearest=jx_palette_nearest(c,mixed[0],mixed[1],mixed[2],palette,threshold);
            assert(nearest>=0); /* Required valid palette; see indeterminate case above. */
            jx_far_set_byte(c,output,pixel,(uint8_t)nearest);
        }
        (void)JX_CALL(c,0x1832,0x1c6c,x,y,width,height,output.offset,output.segment);
    }
    (void)JX_CALL(c,0x1832,0x1a28,x,y,width,height,second.offset,second.segment,255);
}
