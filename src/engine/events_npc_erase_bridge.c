#include "event_engine.h"
#include <assert.h>
static JxFarPointer erase_pointer(const uint16_t *a) {return (JxFarPointer){a[0],a[1]};}
static uint32_t erase_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxNpcEraseBridge *b=opaque;unsigned bit=0;
    if(segment==0x2f28 && offset==0x7a1c) bit=1;
    else if(segment==0x2f28 && offset==0x7978) bit=2;
    else if(segment==0x17d2 && offset==0x01d4) bit=4;
    if(!bit) return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
    assert(!(b->active&bit));b->active|=(uint16_t)bit;
    if(bit==1) {assert(n==1);jx_npc_erase_transition(b->base.context,a[0]);}
    else if(bit==2) {assert(n==1);jx_npc_erase_background(b->base.context,a[0]);}
    else {assert(n==12);jx_transition_blend(b->base.context,a[0],a[1],a[2],a[3],erase_pointer(a+4),erase_pointer(a+6),erase_pointer(a+8),erase_pointer(a+10));}
    b->active&=(uint16_t)~bit;return 0;
}
void jx_npc_erase_bridge_install(JxEventContext *c,JxNpcEraseBridge *b) {
    assert(c && b && c->call && c->call!=erase_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->active=0;c->call=erase_dispatch;c->opaque=b;
}
void jx_npc_erase_bridge_remove(JxEventContext *c,JxNpcEraseBridge *b) {
    assert(c->call==erase_dispatch && c->opaque==b && b->active==0);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
