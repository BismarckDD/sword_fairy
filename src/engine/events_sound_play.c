#include "event_engine.h"

/* 1F2A:14EC. Detect/configure DSP before clearing twelve cached slots. */
void jx_sound_initialize(JxEventContext *c) {
    jx_set_word(c,0x585c,0xfff0);
    (void)JX_CALL(c,0x1f2a,0x2740,0x31b8,c->data_segment);
    (void)JX_CALL(c,0x1f2a,0x266c,0x31b8,c->data_segment);
    (void)JX_CALL(c,0x1f2a,0x2820,jx_word(c,0x31b8));
    for(uint16_t i=0;i<12;i++){
        jx_set_word(c,(uint16_t)(0x28f0+4*i),0);
        jx_set_word(c,(uint16_t)(0x28f2+4*i),0);
    }
}

/* 1F2A:155C. Prepare the cached sound and invoke the original DMA/DSP
 * service boundaries. Success here does not certify audible playback. */
uint16_t jx_sound_play(JxEventContext *c,uint16_t channel) {
    if (!jx_word(c,0x31b6) || !jx_word(c,0x050e) || (int16_t)channel<0) return 0xffff;
    uint16_t entry=(uint16_t)(0x28f0+4u*channel);
    uint16_t descriptor=(uint16_t)(0x2848+10u*channel);
    for (uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(151+slot+jx_word(c,entry)),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x31ca),jx_word(c,0x31cc),
        jx_word(c,0x5858),jx_word(c,0x585a),(uint16_t)(jx_word(c,(uint16_t)(entry+2))+15));
    jx_set_word(c,0xd48e,jx_ds(c)[descriptor]);
    jx_set_word(c,0x3460,jx_ds(c)[(uint16_t)(descriptor+1)]);
    (void)JX_CALL(c,0x1f2a,0x2820,jx_word(c,0x31b8));
    (void)JX_CALL(c,0x1f2a,0x16b0,jx_word(c,0x345c),jx_word(c,0x345e),
        descriptor,c->data_segment,jx_word(c,0x585c));
    (void)JX_CALL(c,0x1f2a,0x18f2,jx_word(c,(uint16_t)(entry+2)),descriptor,c->data_segment,2);
    return 0;
}
/* 1F2A:2820. Port input services return a full AX; poll bit7, then
 * compare the full reset acknowledgement with AAh, returning AL only. */
uint8_t jx_sound_dsp_reset(JxEventContext *c,uint16_t base) {
    (void)JX_CALL(c,0x1000,0x2b5e,(uint16_t)(base+6),1);
    (void)JX_CALL(c,0x1000,0x2b5e,(uint16_t)(base+6),0);
    while (((uint16_t)JX_CALL(c,0x1000,0x2b50,(uint16_t)(base+14))&0x80)==0) {}
    return (uint16_t)JX_CALL(c,0x1000,0x2b50,(uint16_t)(base+10))==0xaa;
}
