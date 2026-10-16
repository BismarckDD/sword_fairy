#include <assert.h>
#include "event_engine.h"
static uint32_t animation_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxAnimationBridge *b=(JxAnimationBridge *)opaque;JxEventContext *c=b->base.context;
    if(segment==0x1000) switch(offset) {
    case 0x4e32:assert(n==4);return jx_animation_open(c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},b->services);
    case 0x4c90:assert(n==4);jx_animation_first(c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},b->services);return 0;
    case 0x4d50:assert(n==4);jx_animation_next(c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},b->services);return 0;
    case 0x4fe0:assert(n==2);jx_animation_close(c,(JxFarPointer){a[0],a[1]},b->services);return 0;
    default:break;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_animation_bridge_install(JxEventContext *c,JxAnimationBridge *b,const JxAnimationServices *services) {
    assert(c && b && c->call && c->call!=animation_dispatch && services);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->services=services;
    c->call=animation_dispatch;c->opaque=b;
}
void jx_animation_bridge_remove(JxEventContext *c,JxAnimationBridge *b) {
    assert(c->call==animation_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
