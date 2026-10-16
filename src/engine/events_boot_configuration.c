#include "event_engine.h"
#include <assert.h>

/* 1F2A:5528. Missing optional configuration leaves all defaults unchanged.
 * Reads may be short; no read/close error changes the original sequence. */
void jx_configuration_load(JxEventContext *c) {
    uint32_t stream=JX_CALL(c,0x1000,0x0738,0x028e,c->data_segment,0x028b,c->data_segment);
    if(!stream)return;
    static const uint16_t words[]={0x31b6,0x31b8,0x31c0,0x31ba,0x3456};
    for(unsigned i=0;i<5;i++)
        (void)JX_CALL(c,0x1000,0x0754,words[i],c->data_segment,2,1,(uint16_t)stream,(uint16_t)(stream>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
    jx_set_word(c,0x31b4,1);
}
static JxBootMemoryFlow allocation_failure(JxEventContext *c,uint16_t message) {
    (void)JX_CALL(c,0x1000,0x0a80,message,c->data_segment);
    (void)JX_CALL(c,0x1000,0x01eb,1);
    return JX_BOOT_MEMORY_EXIT;
}
/* 1F2A:5436. Conventional DOS paragraph allocation is a service boundary.
 * EXIT is termination, never a normal AX error consumed by initialization.
 * Only the low timer word (INT1A DX) is used as the actual RNG seed. */
JxBootMemoryFlow jx_boot_memory_initialize(JxEventContext *c,JxBiosTimerLow ticks,void *opaque) {
    assert(ticks);
    if(jx_signed_word(jx_word(c,0x31b6))>0) {
        if((uint16_t)JX_CALL(c,0x1000,0x32da,0x2000,0x64e2,c->data_segment))
            return allocation_failure(c,0x0263);
        uint16_t segment=(uint16_t)((jx_word(c,0x64e2)&0xf000)+0x1000);
        jx_set_word(c,0x31ca,0);jx_set_word(c,0x31cc,segment);
        uint32_t physical=(uint32_t)segment<<4;
        jx_set_word(c,0x345c,(uint16_t)physical);jx_set_word(c,0x345e,(uint16_t)(physical>>16));
    }
    if((uint16_t)JX_CALL(c,0x1000,0x32da,0x1000,0x64e2,c->data_segment))
        return allocation_failure(c,0x0277);
    jx_set_word(c,0x2eb0,0);jx_set_word(c,0x2eb2,jx_word(c,0x64e2));
    jx_set_word(c,0x888e,0);jx_set_word(c,0x8890,(uint16_t)(jx_word(c,0x64e2)+0x0fa0));
    (void)jx_random_seed(c,ticks(opaque));
    return JX_BOOT_MEMORY_RETURNED;
}
