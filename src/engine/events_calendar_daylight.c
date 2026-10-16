#include "event_engine.h"
/* 1000:2DDA. tm fields are signed words; original CRT uses its historical
 * April/October rules and four-year leap cycle, including the 1987 branch.
 * This is compatibility code, not a current host timezone policy. */
static int tm_field(JxEventContext *c,JxFarPointer p,uint16_t field) {
    return jx_signed_word(jx_far_word(c,p.segment,(uint16_t)(p.offset+field)));
}
static int sar_two(uint16_t bits) {
    int value=jx_signed_word(bits);
    return value>=0?value/4:-((-value+3)/4);
}
uint16_t jx_calendar_is_daylight(JxEventContext *c,JxFarPointer p) {
    int year=tm_field(c,p,10),month=tm_field(c,p,8);
    if(year<67 || month<3 || month>9)return 0;
    if(month>3 && month<9)return 1;
    uint16_t index=(uint16_t)((uint16_t)month*2);
    uint16_t end_day;
    if(year>86 && month==3)end_day=(uint16_t)(jx_word(c,(uint16_t)(0x1712+index))+7);
    else end_day=jx_word(c,(uint16_t)(0x1714+index));
    if(((uint16_t)year&3)==0)end_day++;
    uint16_t days=(uint16_t)((uint16_t)(year-70)*365u);
    days=(uint16_t)(days+sar_two((uint16_t)(year-1))+end_day);
    uint32_t dividend=(uint32_t)(jx_signed_word(days)-13),remainder;
    (void)jx_remainder_signed32(dividend,7,&remainder);
    int transition=jx_signed_word((uint16_t)(end_day-(uint16_t)remainder));
    month=tm_field(c,p,8);
    int yday=tm_field(c,p,14),hour=tm_field(c,p,4);
    if(month==3)return (uint16_t)(yday>transition || (yday==transition && hour>=2));
    return (uint16_t)(yday<transition || (yday==transition && hour<1));
}
