#include "event_engine.h"
#include <assert.h>

static uint32_t voc_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *args,size_t count) {
    JxVocBridge *b=opaque;
    if(segment==0x1f2a && offset==0x1d6e) {
        assert(count==3);
        return jx_voc_load(b->base.context,(JxFarPointer){args[0],args[1]},args[2],b->work);
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
/* This 46-byte scratch allocation must be separate from the resource
 * formatter's 72-byte allocation and other active bridge workspaces. */
void jx_voc_bridge_install(JxEventContext *c,JxVocBridge *b,JxFarPointer work) {
    assert(c && b && c->call && c->call!=voc_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->work=work;c->call=voc_dispatch;c->opaque=b;
}
void jx_voc_bridge_remove(JxEventContext *c,JxVocBridge *b) {
    assert(c->call==voc_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
