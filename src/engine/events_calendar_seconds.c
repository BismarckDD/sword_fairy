#include "event_engine.h"
/* 1000:2C7E. Initialization may mutate the guard itself; increment its live
 * value after the service, preserving word wrap. */
void jx_timezone_ensure_initialized(JxEventContext *c) {
    if(jx_word(c,0x2b8a)==0) {
        (void)JX_CALL0(c,0x1000,0x2c90);
        jx_set_word(c,0x2b8a,(uint16_t)(jx_word(c,0x2b8a)+1));
    }
}
static int quarter_signed(uint16_t bits) {
    int n=jx_signed_word(bits);return n>=0?n/4:-((-n+3)/4);
}
static void calendar_store(JxEventContext *c,JxFarPointer p,uint16_t field,uint16_t value) {
    uint32_t at=(uint32_t)p.segment*16+(uint16_t)(p.offset+field);
    c->memory[at&0xfffff]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 1000:2EA8. Year is relative to1980, month is one-based. tm_work owns18
 * private bytes, disjoint from globals; only four original fields are set.
 * Month table is read before initialization; timezone is read afterwards. */
uint32_t jx_calendar_to_seconds(JxEventContext *c,uint16_t year,uint16_t month,
    uint16_t day,uint16_t hour,uint16_t minute,uint16_t second,JxFarPointer tm_work) {
    uint16_t prior=jx_word(c,(uint16_t)(0x1710+(uint16_t)(month*2)));
    if((year&3)==0 && jx_signed_word(month)>2)prior++;
    (void)JX_CALL0(c,0x1000,0x2c7e);
    uint32_t days=(uint32_t)(jx_signed_word(year)*365+jx_signed_word(day)
        +quarter_signed((uint16_t)(year+3))+jx_signed_word(prior)+0x0e44);
    uint32_t result=jx_multiply_u32(days,24)+(uint32_t)jx_signed_word(hour);
    result=jx_multiply_u32(result,60)+(uint32_t)jx_signed_word(minute);
    result=jx_multiply_u32(result,60)+(uint32_t)jx_signed_word(second);
    result+=jx_word(c,0x1730)|((uint32_t)jx_word(c,0x1732)<<16);
    calendar_store(c,tm_work,14,(uint16_t)(day+prior));
    calendar_store(c,tm_work,10,(uint16_t)(year+0x50));
    calendar_store(c,tm_work,8,(uint16_t)(month-1));
    calendar_store(c,tm_work,4,hour);
    if(jx_word(c,0x1734)!=0 && JX_CALL(c,0x1000,0x2dda,tm_work.offset,tm_work.segment)!=0)result-=3600;
    return result;
}
