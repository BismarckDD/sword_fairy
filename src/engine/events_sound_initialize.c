#include "event_engine.h"

/* 1F2A:2526. Host ISR adapter preserves CPU registers/flags separately. */
void jx_sound_irq(JxEventContext *c) {
    (void)JX_CALL(c,0x1000,0x2b5e,(uint16_t)(jx_word(c,0x31b8)+4),0x82);
    uint8_t status=(uint8_t)JX_CALL(c,0x1000,0x2b50,(uint16_t)(jx_word(c,0x31b8)+5));
    if(status&1)(void)JX_CALL(c,0x1000,0x2b50,(uint16_t)(jx_word(c,0x31b8)+14));
    else if(status&2)(void)JX_CALL(c,0x1000,0x2b50,(uint16_t)(jx_word(c,0x31b8)+15));
    else {
        if(jx_word(c,0x5146) || jx_word(c,0x5148))
            (void)JX_CALL(c,jx_word(c,0x5148),jx_word(c,0x5146));
        return;
    }
    uint32_t remaining=(uint32_t)jx_word(c,0x92b0)|((uint32_t)jx_word(c,0x92b2)<<16);
    remaining-=jx_word(c,0x585c)/2;
    jx_set_word(c,0x92b0,(uint16_t)remaining);jx_set_word(c,0x92b2,(uint16_t)(remaining>>16));
    jx_set_word(c,0x9382,(uint16_t)(jx_word(c,0x9382)^1));
    if(jx_signed_word(jx_word(c,0x31c0))>7)(void)JX_CALL(c,0x1000,0x2b5e,0xa0,0x20);
    (void)JX_CALL(c,0x1000,0x2b5e,0x20,0x20);
}

uint16_t jx_sound_dsp_read(JxEventContext *c,uint16_t base) {
    while(((uint16_t)JX_CALL(c,0x1000,0x2b50,(uint16_t)(base+14))&0x80)==0){}
    return (uint16_t)JX_CALL(c,0x1000,0x2b50,(uint16_t)(base+10));
}
/* 1F2A:2740. Reset uses the live global base; subsequent operations use
 * the caller's configuration pointer, reread after each device callback. */
uint8_t jx_sound_detect_version(JxEventContext *c,JxFarPointer config) {
    (void)JX_CALL(c,0x1f2a,0x2820,jx_word(c,0x31b8));
    (void)JX_CALL(c,0x1f2a,0x27e4,jx_far_word(c,config.segment,config.offset),0xe1);
    uint16_t major=(uint16_t)JX_CALL(c,0x1f2a,0x27aa,jx_far_word(c,config.segment,config.offset));
    uint16_t minor=(uint16_t)JX_CALL(c,0x1f2a,0x27aa,jx_far_word(c,config.segment,config.offset));
    uint16_t version=(uint16_t)((major<<8)|minor);
    jx_far_set_byte(c,config,6,(uint8_t)version);jx_far_set_byte(c,config,7,(uint8_t)(version>>8));
    return 1;
}
static uint16_t irq_number(JxEventContext *c,JxFarPointer config) {
    return jx_far_word(c,config.segment,(uint16_t)(config.offset+8));
}
static uint16_t mask_bit(uint16_t bit) {
    unsigned shift=bit&31;return shift<16?(uint16_t)(UINT16_C(1)<<shift):0;
}
/* 1F2A:266C. PIC masks and old interrupt vector are retained for cleanup.
 * The host supplies CLI/STI effects separately from engine service calls. */
void jx_sound_install_irq(JxEventContext *c,JxFarPointer config,
    void (*gate)(void *,uint8_t),void *opaque) {
    if(gate)gate(opaque,0);
    jx_set_word(c,0x5862,(uint16_t)JX_CALL(c,0x1000,0x2b50,0x21));
    uint16_t vector;
    if(jx_signed_word(irq_number(c,config))>7){
        vector=(uint16_t)(irq_number(c,config)+0x68);
        jx_set_word(c,0x5864,(uint16_t)JX_CALL(c,0x1000,0x2b50,0xa1));
        uint16_t mask=(uint16_t)~mask_bit((uint16_t)(irq_number(c,config)-8));
        (void)JX_CALL(c,0x1000,0x2b5e,0xa1,(uint16_t)(jx_word(c,0x5864)&mask));
        (void)JX_CALL(c,0x1000,0x2b5e,0x21,(uint16_t)(jx_word(c,0x5862)&0xfffb));
    }else{
        vector=(uint16_t)(irq_number(c,config)+8);
        uint16_t mask=(uint16_t)~mask_bit(irq_number(c,config));
        (void)JX_CALL(c,0x1000,0x2b5e,0x21,(uint16_t)(jx_word(c,0x5862)&mask));
    }
    uint32_t old=JX_CALL(c,0x1000,0x3302,vector);
    jx_set_word(c,0x5146,(uint16_t)old);jx_set_word(c,0x5148,(uint16_t)(old>>16));
    (void)JX_CALL(c,0x1000,0x3314,vector,0x2526,0x1f2a);
    if(gate)gate(opaque,1);
}
