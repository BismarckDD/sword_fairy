#include "event_engine.h"
#include <assert.h>
static uint32_t port_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxPortBridge *b=opaque;
    if(segment==0x1000) {
        if(offset==0x2b50) {assert(n==1);return jx_port_read_byte(&b->device,a[0]);}
        if(offset==0x2b5e) {assert(n==2);return jx_port_write_byte(&b->device,a[0],a[1]);}
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_port_bridge_install(JxEventContext *c,JxPortBridge *b,const JxPortDevice *device) {
    assert(c && b && device && device->read && device->write && c->call && c->call!=port_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->device=*device;c->call=port_dispatch;c->opaque=b;
}
void jx_port_bridge_remove(JxEventContext *c,JxPortBridge *b) {
    assert(c->call==port_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
