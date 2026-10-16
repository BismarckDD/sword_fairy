#include "event_engine.h"
static uint32_t timezone_value(JxEventContext *c) {return jx_word(c,0x1730)|((uint32_t)jx_word(c,0x1732)<<16);}
static void timezone_store(JxEventContext *c,uint32_t value) {jx_set_word(c,0x1730,(uint16_t)value);jx_set_word(c,0x1732,(uint16_t)(value>>16));}
static int timezone_digit(uint8_t ch) {return ch>='0' && ch<='9';}
static uint32_t timezone_decimal(JxEventContext *c,JxFarPointer p) {return JX_CALL(c,0x1000,0x252e,p.offset,p.segment);}
/* 1000:2C90. Interpret the DOS CRT TZ environment syntax exactly, with
 * fixed-offset wrap and live name destinations/timezone between calls. */
void jx_timezone_initialize(JxEventContext *c) {
    uint32_t value=JX_CALL(c,0x1000,0x26a4,0x172c,c->data_segment);
    JxFarPointer p={(uint16_t)value,(uint16_t)(value>>16)};
    if((p.offset|p.segment)==0 || jx_far_byte(c,p,0)==0)return;
    (void)JX_CALL(c,0x1000,0x24c8,jx_word(c,0x173e),jx_word(c,0x1740),p.offset,p.segment,3);
    p.offset=(uint16_t)(p.offset+3);unsigned negative=0;
    if(jx_far_byte(c,p,0)=='-') {negative=1;p.offset++;}
    timezone_store(c,jx_multiply_u32(timezone_decimal(c,p),3600));
    while(jx_far_byte(c,p,0)=='+' || timezone_digit(jx_far_byte(c,p,0)))p.offset++;
    if(jx_far_byte(c,p,0)==':') {
        p.offset++;
        uint32_t minutes=jx_multiply_u32(timezone_decimal(c,p),60);
        timezone_store(c,timezone_value(c)+minutes);
        while(timezone_digit(jx_far_byte(c,p,0)))p.offset++;
        if(jx_far_byte(c,p,0)==':') {
            p.offset++;uint32_t seconds=timezone_decimal(c,p);
            timezone_store(c,timezone_value(c)+seconds);
            while(timezone_digit(jx_far_byte(c,p,0)))p.offset++;
        }
    }
    if(negative)timezone_store(c,0u-timezone_value(c));
    uint8_t ch=jx_far_byte(c,p,0);
    jx_set_word(c,0x1734,(uint16_t)(ch<128?ch:(int)ch-256));
    if(ch!=0)(void)JX_CALL(c,0x1000,0x24c8,jx_word(c,0x1742),jx_word(c,0x1744),p.offset,p.segment,3);
    else {
        JxFarPointer dest={jx_word(c,0x1742),jx_word(c,0x1744)};
        jx_far_set_byte(c,dest,0,0);
    }
}
