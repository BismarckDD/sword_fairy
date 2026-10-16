#include "event_engine.h"

static uint16_t arithmetic_flags(uint16_t prior,uint16_t left,uint16_t right,uint16_t result) {
    unsigned bits=0;for(unsigned v=result&255;v;v>>=1)bits+=v&1;
    return (uint16_t)((prior&~0x08d5u)|(left<right?1:0)|((bits&1)?0:4)|
        ((left^right^result)&0x10)|(result?0:0x40)|(result&0x8000?0x80:0)|
        (((left^right)&(left^result)&0x8000)?0x800:0));
}
static void stack_word(JxEventContext *c,uint16_t segment,uint16_t offset,uint16_t value) {
    JxFarPointer at={offset,segment};
    jx_far_set_byte(c,at,0,(uint8_t)value);jx_far_set_byte(c,at,1,(uint8_t)(value>>8));
}
/* 1000:02C6. Original far-call stack allocation/probe, including return-frame
 * relocation and both failure transfers. This is an explicit semantic API:
 * native C callers use their own ABI; registered handlers and1000:00F4 are
 * returned as continuations rather than called with an invalid C stack. */
JxStackProbeResult jx_stack_probe(JxEventContext *c,JxFarPointer stack,uint16_t requested,uint16_t flags) {
    uint16_t ip=jx_far_word(c,stack.segment,stack.offset);
    uint16_t cs=jx_far_word(c,stack.segment,(uint16_t)(stack.offset+2));
    uint16_t post=(uint16_t)(stack.offset+4),next=(uint16_t)(post-requested);
    JxStackProbeResult result={JX_STACK_RETURNED,requested,next,ip,cs,next,0,{ip,cs}};
    flags=arithmetic_flags(flags,post,requested,next);
    int failure=post<requested;
    if(!failure){
        uint16_t limit=jx_word(c,0x1352);
        flags=arithmetic_flags(flags,next,limit,(uint16_t)(next-limit));failure=next<limit;
    }
    if(!failure){
        stack_word(c,stack.segment,(uint16_t)(next-2),cs);
        stack_word(c,stack.segment,(uint16_t)(next-4),ip);
        result.flags=flags;return result;
    }
    stack_word(c,stack.segment,(uint16_t)(post-2),cs);
    stack_word(c,stack.segment,(uint16_t)(post-4),ip);
    result.sp=stack.offset;
    uint16_t handler=jx_word(c,0x134e),value=(uint16_t)(handler+1);
    uint16_t increment=arithmetic_flags(flags,handler,0xffff,value);
    /* INC preserves CF and uses addition overflow rather than SUB overflow. */
    increment=(uint16_t)((increment&~0x811u)|(flags&1)|((handler^1^value)&0x10)|(handler==0x7fff?0x800:0));
    result.ax=value;result.flags=increment;
    if(value){
        result.flow=JX_STACK_HANDLER;result.target=(JxFarPointer){handler,jx_word(c,0x1350)};
    }else{
        result.flow=JX_STACK_STARTUP_ERROR;result.ax=0;
        result.flags=(uint16_t)((increment&~0x8d5u)|0x44);
        result.target=(JxFarPointer){0x00f4,0x1000};
    }
    return result;
}
