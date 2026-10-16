#include "event_engine.h"

/* 1000:341E. Compiler helper returns the low 32 bits; signed and unsigned
 * products have the same low bits. Its original callee pops eight bytes. */
uint32_t jx_multiply_u32(uint32_t left,uint32_t right) {return left*right;}

/* 1000:3384. Magnitudes avoid host signed overflow, including INT_MIN/-1.
 * A zero denominator faults in the original; report it without inventing a
 * quotient. The bridge delegates that exceptional path to the host. */
int jx_divide_signed32(uint32_t numerator,uint32_t denominator,uint32_t *quotient) {
    if(!denominator) return 0;
    unsigned negative=(numerator>>31)^(denominator>>31);
    uint32_t a=(numerator>>31)?0u-numerator:numerator;
    uint32_t b=(denominator>>31)?0u-denominator:denominator;
    uint32_t q=a/b;
    *quotient=negative?0u-q:q;
    return 1;
}

/* 1000:30EA, DF=0. Unsigned byte comparison with huge-pointer carries;
 * embedded zero bytes have no special meaning. */
int16_t jx_memory_compare(JxEventContext *c,JxFarPointer left,JxFarPointer right,uint16_t count) {
    for(unsigned i=0;i<count;i++) {
        uint8_t a=jx_far_byte(c,left,0),b=jx_far_byte(c,right,0);
        if(a!=b) return a<b?-1:1;
        if(++left.offset==0) left.segment=(uint16_t)(left.segment+0x1000);
        if(++right.offset==0) right.segment=(uint16_t)(right.segment+0x1000);
    }
    return 0;
}

/* 1832:185E, DF=0. The original enters LOOP without testing CX, so zero
 * means 65536 pixels. Both offsets wrap without advancing their segments. */
void jx_memory_copy_transparent(JxEventContext *c,JxFarPointer destination,
    JxFarPointer source,uint16_t count,uint16_t transparent) {
    unsigned pixels=count?count:65536u;
    for(unsigned i=0;i<pixels;i++) {
        uint8_t value=jx_far_byte(c,source,(uint16_t)i);
        if(value!=(uint8_t)transparent) jx_far_set_byte(c,destination,(uint16_t)i,value);
    }
}

/* 1000:32A0/32B2. Keep the seed in the original data words, so memory
 * snapshots and callers accessing the CRT state see the same sequence. */
uint16_t jx_random_seed(JxEventContext *c,uint16_t seed) {
    jx_set_word(c,0x1752,seed);jx_set_word(c,0x1754,0);return seed;
}
uint16_t jx_random_next(JxEventContext *c) {
    uint32_t state=jx_word(c,0x1752)|((uint32_t)jx_word(c,0x1754)<<16);
    state=state*UINT32_C(214013)+UINT32_C(2531011);
    jx_set_word(c,0x1752,(uint16_t)state);jx_set_word(c,0x1754,(uint16_t)(state>>16));
    return (uint16_t)((state>>16)&0x7fff);
}

/* The original string routines scan at most 65535 bytes with a fixed
 * segment. Copying also wraps only offsets, unlike huge memory_copy. */
static unsigned scanned_string(JxEventContext *c,JxFarPointer p,int *found) {
    for(unsigned count=1;count<=65535;count++) {
        if(jx_far_byte(c,p,(uint16_t)(count-1))==0) {*found=1;return count;}
    }
    *found=0;return 65535;
}
/* 1000:24AE. Unterminated strings return 65535 after the bounded scan. */
uint16_t jx_string_length(JxEventContext *c,JxFarPointer text) {
    int found;unsigned count=scanned_string(c,text,&found);
    return (uint16_t)(count-(found?1u:0u));
}
static uint32_t string_result(JxFarPointer p) {return ((uint32_t)p.segment<<16)|p.offset;}
static void transfer_byte(JxEventContext *c,JxFarPointer *destination,JxFarPointer *source) {
    jx_far_set_byte(c,*destination,0,jx_far_byte(c,*source,0));
    destination->offset++;source->offset++;
}
static void transfer_word(JxEventContext *c,JxFarPointer *destination,JxFarPointer *source) {
    uint16_t value=jx_far_word(c,source->segment,source->offset);
    uint32_t address=((uint32_t)destination->segment*16+destination->offset)&0xfffff;
    c->memory[address]=(uint8_t)value;c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
    destination->offset=(uint16_t)(destination->offset+2);source->offset=(uint16_t)(source->offset+2);
}
static void transfer_tail(JxEventContext *c,JxFarPointer destination,JxFarPointer source,unsigned count) {
    for(unsigned i=0;i<count/2;i++) transfer_word(c,&destination,&source);
    if(count%2) transfer_byte(c,&destination,&source);
}

/* 1000:2448. Word grouping is aligned by DESTINATION parity. The unusual
 * unterminated-source path copies 65536 bytes; retain its bounded behavior. */
uint32_t jx_string_copy(JxEventContext *c,JxFarPointer destination,JxFarPointer source) {
    uint32_t result=string_result(destination);int found;
    unsigned count=scanned_string(c,source,&found);
    if(!found) {transfer_word(c,&destination,&source);count--;}
    else if(destination.offset&1) {transfer_byte(c,&destination,&source);count--;}
    transfer_tail(c,destination,source,count);
    return result;
}

/* 1000:23F4. Append instead aligns word grouping by SOURCE parity. */
uint32_t jx_string_append(JxEventContext *c,JxFarPointer destination,JxFarPointer source) {
    uint32_t result=string_result(destination);int found;
    unsigned end=scanned_string(c,destination,&found);
    destination.offset=(uint16_t)(destination.offset+end-1);
    unsigned count=scanned_string(c,source,&found);
    if(!found) {transfer_word(c,&destination,&source);count=65534;}
    else if(source.offset&1) {transfer_byte(c,&destination,&source);count--;}
    transfer_tail(c,destination,source,count);
    return result;
}
/* 1000:2FFC. ASCII-only in-place uppercase; fixed segment offset wraps.
 * Like the original this requires a NUL within the 64KiB segment cycle. */
uint32_t jx_string_uppercase(JxEventContext *c,JxFarPointer text) {
    JxFarPointer p=text;
    for(uint8_t value;(value=jx_far_byte(c,p,0))!=0;p.offset++)
        if(value>='a' && value<='z') jx_far_set_byte(c,p,0,(uint8_t)(value-'a'+'A'));
    return string_result(text);
}
/* 1000:2484, DF=0. Scan the right operand up to65535 bytes, then compare
 * that many bytes including NUL; the original returns only -1,0,+1. */
int16_t jx_string_compare(JxEventContext *c,JxFarPointer left,JxFarPointer right) {
    int found;unsigned count=scanned_string(c,right,&found);(void)found;
    for(unsigned i=0;i<count;i++) {
        uint8_t a=jx_far_byte(c,left,(uint16_t)i),b=jx_far_byte(c,right,(uint16_t)i);
        if(a!=b) return a<b?-1:1;
    }
    return 0;
}
