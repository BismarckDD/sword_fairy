#include "event_engine.h"
static void ftime_word(JxEventContext *c,JxFarPointer p,uint16_t field,uint16_t value) {
    uint32_t at=(uint32_t)p.segment*16+(uint16_t)(p.offset+field);
    c->memory[at&0xfffff]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 1000:2B6C. Output10 bytes; private_work owns34 bytes: date5,time4,
 * and tm18 at offsets0,8,16. They must not alias output or globals.
 * DOS calendar, initialization and conversion remain address callbacks. */
uint16_t jx_ftime(JxEventContext *c,JxFarPointer output,JxFarPointer private_work) {
    JxFarPointer date=private_work;
    JxFarPointer time={(uint16_t)(private_work.offset+8),private_work.segment};
    JxFarPointer tm={(uint16_t)(private_work.offset+16),private_work.segment};
    (void)JX_CALL0(c,0x1000,0x2c7e);
    uint32_t timezone=jx_word(c,0x1730)|((uint32_t)jx_word(c,0x1732)<<16);
    uint32_t minutes=JX_CALL(c,0x1000,0x3384,(uint16_t)timezone,(uint16_t)(timezone>>16),60,0);
    ftime_word(c,output,6,(uint16_t)minutes);
    (void)JX_CALL(c,0x1000,0x332a,date.offset,date.segment);
    (void)JX_CALL(c,0x1000,0x3346,time.offset,time.segment);
    if(jx_far_byte(c,time,0)==0 && jx_far_byte(c,time,1)==0)
        (void)JX_CALL(c,0x1000,0x332a,date.offset,date.segment);
    uint16_t year=(uint16_t)(jx_far_word(c,date.segment,(uint16_t)(date.offset+2))-1980);
    uint16_t day=jx_far_byte(c,date,0),month=jx_far_byte(c,date,1);
    uint16_t yday=(uint16_t)(day+jx_word(c,(uint16_t)(0x1712+(uint16_t)((month-1)*2))));
    if((year&3)==0 && month>2)yday++;
    ftime_word(c,tm,10,(uint16_t)(year+80));ftime_word(c,tm,6,day);
    ftime_word(c,tm,8,(uint16_t)(month-1));ftime_word(c,tm,14,yday);
    ftime_word(c,tm,4,jx_far_byte(c,time,0));
    ftime_word(c,output,4,(uint16_t)(jx_far_byte(c,time,3)*10));
    uint32_t seconds=JX_CALL(c,0x1000,0x2ea8,year,month,day,
        jx_far_byte(c,time,0),jx_far_byte(c,time,1),jx_far_byte(c,time,2));
    ftime_word(c,output,0,(uint16_t)seconds);ftime_word(c,output,2,(uint16_t)(seconds>>16));
    uint16_t result=(uint16_t)seconds,daylight=0;
    if(jx_word(c,0x1734)!=0) {
        result=(uint16_t)JX_CALL(c,0x1000,0x2dda,tm.offset,tm.segment);
        if(result!=0)daylight=1;
    }
    ftime_word(c,output,8,daylight);return result;
}
