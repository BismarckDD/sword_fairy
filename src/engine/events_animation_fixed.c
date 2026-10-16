#include "event_engine.h"
static uint16_t fixed_animation_page(JxEventContext *c) {
    uint16_t next=(uint16_t)(jx_word(c,0x3e54)+1);
    return (uint16_t)(jx_signed_word(next)%2);
}
/* 2F28:6F12. Caller-supplied frame delay; no rate division, cue table,
 * subtitle overlay or palette gain conversion from the 7062 player. */
void jx_animation_play_fixed_delay(JxEventContext *c,JxFarPointer filename,uint16_t delay,uint16_t fade) {
    for(uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(179+slot),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1000,0x4e32,filename.offset,filename.segment,0x3542,c->data_segment);
    if(fade) (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    for(uint16_t frame=0;frame<jx_word(c,0x3548);frame++) {
        (void)JX_CALL(c,0x1832,0x1810,fixed_animation_page(c));
        (void)JX_CALL(c,0x1000,0x4d50,0x3542,c->data_segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        (void)JX_CALL(c,0x1832,0x1836,fixed_animation_page(c));
        if(jx_signed_word(fade)>0 && frame==0)
            (void)JX_CALL(c,0x2f28,0x415e,0x354c,c->data_segment,1);
        jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
        (void)JX_CALL(c,0x1f2a,0x720e,delay);
    }
    (void)JX_CALL(c,0x1000,0x4fe0,0x3542,c->data_segment);
    if(fade) (void)JX_CALL(c,0x2f28,0x3f66,0x354c,c->data_segment,1);
}
