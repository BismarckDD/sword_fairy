#include "event_engine.h"
#include <string.h>

/* 2F28:6C7E. Remap indexed pixels through a caller-supplied 256-byte table.
 * The optional palette refresh precedes even a zero-length transfer. Its
 * source palette's first four bytes are passed by value to A8D0, while the
 * output palette-index table is always the global DS:B584 in that call.
 * The actual pixel loop still uses the table supplied by this caller.
 * IEEE float conversion assumes the normal round-to-nearest environment;
 * exceptional FP values and the original software FP handler are unverified.
 */
void jx_color_map_pixels(JxEventContext *c,JxFarPointer source,
    JxFarPointer destination,uint16_t count,JxFarPointer table) {
    if(jx_signed_word(jx_word(c,0x6626))>0) {
        uint64_t bits=0;
        for(unsigned i=0;i<8;i++) bits|=(uint64_t)jx_ds(c)[0x23a2+i]<<(i*8);
        double original;float weight;uint32_t argument;
        _Static_assert(sizeof(original)==8 && sizeof(weight)==4,"IEEE floating sizes required");
        memcpy(&original,&bits,sizeof(original));weight=(float)original;
        memcpy(&argument,&weight,sizeof(argument));
        (void)JX_CALL(c,0x2f28,0xa8d0,0x61de,c->data_segment,
            jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,
            (uint16_t)argument,(uint16_t)(argument>>16));
    }
    for(uint16_t pixel=0;pixel<count;pixel++) {
        uint8_t color=jx_far_byte(c,source,pixel);
        jx_far_set_byte(c,destination,pixel,jx_far_byte(c,table,color));
    }
}
