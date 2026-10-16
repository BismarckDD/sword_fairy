#include "event_engine.h"
/* 1000:332A. Day/month/year/weekday use the original five-byte record. */
uint16_t jx_dos_get_date(JxEventContext *c,JxFarPointer p,JxDosCalendarService service,void *opaque) {
    JxDosCalendarRegisters r={0,0,0};service(opaque,0x2a,&r);
    uint16_t at=(uint16_t)(p.offset+2);
    c->memory[((uint32_t)p.segment*16+at)&0xfffff]=(uint8_t)r.cx;
    c->memory[((uint32_t)p.segment*16+at+1)&0xfffff]=(uint8_t)(r.cx>>8);
    jx_far_set_byte(c,p,1,(uint8_t)(r.dx>>8));
    jx_far_set_byte(c,p,0,(uint8_t)r.dx);
    jx_far_set_byte(c,p,4,(uint8_t)r.ax);
    return 0;
}
/* 1000:3346. Hour/minute/second/hundredths, ignoring DOS flags. */
uint16_t jx_dos_get_time(JxEventContext *c,JxFarPointer p,JxDosCalendarService service,void *opaque) {
    JxDosCalendarRegisters r={0,0,0};service(opaque,0x2c,&r);
    jx_far_set_byte(c,p,0,(uint8_t)(r.cx>>8));
    jx_far_set_byte(c,p,1,(uint8_t)r.cx);
    jx_far_set_byte(c,p,2,(uint8_t)(r.dx>>8));
    jx_far_set_byte(c,p,3,(uint8_t)r.dx);
    return 0;
}
