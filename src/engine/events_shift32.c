#include "event_engine.h"
/* 1000:34F0 / 34FC / 3508. The register ABI uses DX:AX and CL;
 * CH is cleared before a loop of single-bit shifts, so counts are not
 * masked to five bits. Explicit guards avoid C shifts by 32 or more. */
uint32_t jx_shift_left32(uint32_t value,uint16_t count) {
    unsigned n=(uint8_t)count;
    return n>=32?0:value<<n;
}
uint32_t jx_shift_right_unsigned32(uint32_t value,uint16_t count) {
    unsigned n=(uint8_t)count;
    return n>=32?0:value>>n;
}
uint32_t jx_shift_right_signed32(uint32_t value,uint16_t count) {
    unsigned n=(uint8_t)count;
    if(!n) return value;
    uint32_t sign=(value&0x80000000u)?0xffffffffu:0;
    if(n>=32) return sign;
    return (value>>n)|(sign<<(32-n));
}
