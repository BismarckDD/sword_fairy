#include "event_engine.h"
#include <assert.h>

static uint32_t cutscene_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *args,size_t count) {
    JxCutsceneBridge *b=opaque;
    if(segment==0x2f28 && offset==0x3b8e){assert(count==2);jx_ending_animation(b->base.context,(JxFarPointer){args[0],args[1]},b->event_work);return 0;}
    if (segment==0x2f28 && offset==0x373e) {
        assert(count==0);jx_lake_cutscene(b->base.context,b->event_work);return 0;
    }
    if (segment==0x2f28 && offset==0x7ec8) {
        assert(count==0);jx_credits_run(b->base.context,b->event_work);return 0;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
/* Caller supplies eight private bytes, outside live game state and buffers.
 * Input timing remains a host service; this bridge performs no polling itself. */
void jx_cutscene_bridge_install(JxEventContext *c,JxCutsceneBridge *b,JxFarPointer event_work) {
    assert(c && b && c->call && c->call!=cutscene_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->event_work=event_work;c->call=cutscene_dispatch;c->opaque=b;
}
void jx_cutscene_bridge_remove(JxEventContext *c,JxCutsceneBridge *b) {
    assert(c->call==cutscene_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
