#include "event_engine.h"
#include <string.h>

/* 1F2A:58FA. Small image data after the header shares the final part
 * of the graphics allocation. Open failure and short reads are optional. */
void jx_image_tail_load(JxEventContext *c,JxFarPointer filename) {
    uint32_t file=JX_CALL(c,0x1000,0x738,filename.offset,filename.segment,0x02a7,c->data_segment);
    if(!file)return;
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),16,0,0);
    (void)JX_CALL(c,0x1000,0x754,jx_word(c,0x888e),jx_word(c,0x8890),1,860,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
}

/* 2F28:6D18. Read the IMG pixels/palette after its 16-byte header.
 * Failed seeks and short reads are deliberately ignored by the original. */
void jx_image_present(JxEventContext *c,JxFarPointer filename) {
    uint32_t file=JX_CALL(c,0x1000,0x738,filename.offset,filename.segment,0x0de3,c->data_segment);
    if (!file) return;
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),16,0,0);
    (void)JX_CALL(c,0x1000,0x754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,64000,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x754,0x2eb4,c->data_segment,3,256,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    (void)JX_CALL(c,0x1832,0x1a28,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2),255);
    (void)JX_CALL(c,0x150f,6,0x2eb4,c->data_segment,0,256);
}

/* 2F28:6DFE. Opaque background variant: load before fading, then fade
 * in and construct the palette translation table from the live state. */
void jx_image_background_present(JxEventContext *c,JxFarPointer filename) {
    uint32_t file=JX_CALL(c,0x1000,0x738,filename.offset,filename.segment,0x0de6,c->data_segment);
    if (!file) return;
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),16,0,0);
    (void)JX_CALL(c,0x1000,0x754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,64000,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x754,0x2eb4,c->data_segment,3,256,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    uint64_t bits=0;
    JxFarPointer weight_address={0x23a2,c->data_segment};
    for(unsigned i=0;i<8;i++) bits|=(uint64_t)jx_far_byte(c,weight_address,(uint16_t)i)<<(8*i);
    double weight_double; memcpy(&weight_double,&bits,sizeof(weight_double));
    float weight=(float)weight_double; uint32_t weight_bits;
    memcpy(&weight_bits,&weight,sizeof(weight_bits));
    (void)JX_CALL(c,0x2f28,0xa8d0,0x2eb4,c->data_segment,jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,(uint16_t)weight_bits,(uint16_t)(weight_bits>>16));
}
