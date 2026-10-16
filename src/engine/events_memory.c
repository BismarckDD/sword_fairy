#include "event_engine.h"

static uint32_t pointer_result(JxFarPointer p) {
    return ((uint32_t)p.segment<<16)|p.offset;
}
static uint32_t physical(JxFarPointer p) {
    return ((uint32_t)p.segment*16+p.offset)&0xfffff;
}
static void advance(JxFarPointer *p,unsigned count) {
    unsigned sum=p->offset+count;
    p->offset=(uint16_t)sum;
    p->segment=(uint16_t)(p->segment+(sum>>16)*4096u);
}

/* 1000:3146. Huge-pointer forward copy, with word transfers followed by
 * an odd byte at each segment boundary. Preserve that order for aliases;
 * host memcpy/memmove would not reproduce overlapping original buffers.
 * The caller follows the original C ABI's clear direction flag. */
uint32_t jx_memory_copy(JxEventContext *c,JxFarPointer destination,
    JxFarPointer source,uint16_t count) {
    uint32_t result=pointer_result(destination);
    unsigned remaining=count;
    while(remaining) {
        unsigned chunk=remaining;
        unsigned source_room=65536u-source.offset;
        unsigned destination_room=65536u-destination.offset;
        if(chunk>source_room) chunk=source_room;
        if(chunk>destination_room) chunk=destination_room;
        unsigned words=chunk/2;
        for(unsigned i=0;i<words;i++) {
            uint32_t from=physical(source),to=physical(destination);
            uint8_t low=c->memory[from],high=c->memory[(from+1)&0xfffff];
            c->memory[to]=low;c->memory[(to+1)&0xfffff]=high;
            advance(&source,2);advance(&destination,2);
        }
        if(chunk%2) {
            c->memory[physical(destination)]=c->memory[physical(source)];
            advance(&source,1);advance(&destination,1);
        }
        remaining-=chunk;
    }
    return result;
}

/* 1000:31A4. Fill uses only the low byte of the value argument and continues
 * through a 64-KiB offset wrap by advancing the segment by 0x1000. */
uint32_t jx_memory_fill(JxEventContext *c,JxFarPointer destination,
    uint16_t value,uint16_t count) {
    uint32_t result=pointer_result(destination),address=physical(destination);
    for(unsigned i=0;i<count;i++) c->memory[(address+i)&0xfffff]=(uint8_t)value;
    return result;
}
