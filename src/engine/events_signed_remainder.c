#include "event_engine.h"
/* 1000:3450. Bit patterns preserve INT_MIN and avoid C signed overflow.
 * A zero divisor represents original INT0; result is left untouched. */
int jx_remainder_signed32(uint32_t numerator,uint32_t denominator,uint32_t *result) {
    if(denominator==0)return 0;
    uint32_t magnitude=(numerator&0x80000000u)?0u-numerator:numerator;
    uint32_t divisor=(denominator&0x80000000u)?0u-denominator:denominator;
    uint32_t remainder=magnitude%divisor;
    *result=(numerator&0x80000000u)?0u-remainder:remainder;
    return 1;
}
