#include "event_engine.h"
#include <assert.h>
static JxFarPointer pointer(const uint16_t *args) {return (JxFarPointer){args[0],args[1]};}
static uint32_t ems_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    JxEmsBridge *b=opaque;JxEventContext *c=b->base.context;
    if(segment==0x1503) {
        switch(offset) {
        case 0x0006:assert(count==0);return jx_ems_status(&b->device);
        case 0x001d:assert(count==2);return jx_ems_frame(c,&b->device,pointer(args));
        case 0x003a:assert(count==4);return jx_ems_page_counts(c,&b->device,pointer(args),pointer(args+2));
        case 0x005d:assert(count==3);return jx_ems_allocate(c,&b->device,args[0],pointer(args+1));
        case 0x007d:assert(count==3);return jx_ems_map(&b->device,args[0],args[1],args[2]);
        case 0x009f:assert(count==1);return jx_ems_release(&b->device,args[0]);
        default:break;
        }
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
void jx_ems_bridge_install(JxEventContext *c,JxEmsBridge *b,const JxEmsDevice *device) {
    assert(c && b && device && device->service && c->call && c->call!=ems_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->device=*device;
    c->call=ems_dispatch;c->opaque=b;
}
void jx_ems_bridge_remove(JxEventContext *c,JxEmsBridge *b) {
    assert(c->opaque==b && c->call==ems_dispatch);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
