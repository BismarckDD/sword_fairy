#include "event_engine.h"

/* 1000:2FA0. ftime_work owns ten private bytes, disjoint from live globals.
 * Ftime/calendar acquisition stays an explicit 2B6C service until restored. */
uint32_t jx_clock_milliseconds(JxEventContext *c,JxFarPointer ftime_work) {
    jx_set_word(c,0x1746,0);
    (void)JX_CALL(c,0x1000,0x2b6c,ftime_work.offset,ftime_work.segment);
    uint32_t seconds=jx_far_word(c,ftime_work.segment,ftime_work.offset)
        |((uint32_t)jx_far_word(c,ftime_work.segment,(uint16_t)(ftime_work.offset+2))<<16);
    uint32_t origin=jx_word(c,0x1748)|((uint32_t)jx_word(c,0x174a)<<16);
    uint32_t result=jx_multiply_u32(seconds-origin,1000);
    uint16_t milliseconds=jx_far_word(c,ftime_work.segment,(uint16_t)(ftime_work.offset+4));
    int fraction=jx_signed_word((uint16_t)(milliseconds-jx_word(c,0x174c)));
    return result+(uint32_t)fraction;
}
/* 1F2A:720E. The first clock sample is retained; elapsed difference wraps
 * as a signed 32-bit word pair. Equality keeps waiting. No input is polled. */
void jx_wait_milliseconds(JxEventContext *c,uint16_t duration) {
    if(jx_signed_word(duration)<=0)return;
    uint32_t start=JX_CALL0(c,0x1000,0x2fa0);
    for(;;) {
        uint32_t elapsed=JX_CALL0(c,0x1000,0x2fa0)-start;
        if((elapsed&0x80000000u)==0 && elapsed>duration)return;
    }
}
