#include "event_engine.h"
/* 1F2A:966E. Scan in order: an empty signed-byte slot is filled before
 * any later duplicate is examined. Positive word IDs truncate to a byte;
 * even 80..7FFF are accepted, and may create another negative empty slot. */
uint16_t jx_actor_skill_grant(JxEventContext *c,uint16_t actor,uint16_t skill) {
    if(jx_signed_word(actor)<0 || jx_signed_word(skill)<0) return 0xffff;
    uint16_t base=(uint16_t)(0xd082+actor*68u);
    for(unsigned slot=0;slot<16;slot++) {
        uint16_t at=(uint16_t)(base+slot);
        int value=jx_signed_byte(jx_ds(c)[at]);
        if(value==(int)skill) return 0;
        if(value<0) {jx_ds(c)[at]=(uint8_t)skill;return 0;}
    }
    return 1;
}
