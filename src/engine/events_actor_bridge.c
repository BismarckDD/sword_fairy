#include "event_engine.h"
#include <assert.h>

static uint32_t actor_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *args,size_t count) {
    JxActorBridge *b=opaque;
    if (segment==0x1f2a && offset==0x747c) {
        assert(count==1);jx_actor_restore_current(b->base.context,args[0]);return 0;
    }
    if (segment==0x1f2a && offset==0x43b2) {
        assert(count==2);jx_actor_level_up(b->base.context,args[0],args[1],b->text_work);return 0;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
void jx_actor_bridge_install(JxEventContext *c,JxActorBridge *b,JxFarPointer text_work) {
    assert(c && b && c->call && c->call!=actor_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->text_work=text_work;c->call=actor_dispatch;c->opaque=b;
}
void jx_actor_bridge_remove(JxEventContext *c,JxActorBridge *b) {
    assert(c->call==actor_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
