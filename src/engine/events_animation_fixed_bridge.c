#include "event_engine.h"
#include <assert.h>
static uint32_t fixed_player_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxFixedAnimationBridge *b=opaque;
    if(segment!=0x2f28 || offset!=0x6f12)
        return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
    assert(n==4 && b->active==0);b->active=1;
    jx_animation_play_fixed_delay(b->base.context,(JxFarPointer){a[0],a[1]},a[2],a[3]);
    b->active=0;return 0;
}
void jx_animation_fixed_bridge_install(JxEventContext *c,JxFixedAnimationBridge *b) {
    assert(c && b && c->call && c->call!=fixed_player_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->active=0;c->call=fixed_player_dispatch;c->opaque=b;
}
void jx_animation_fixed_bridge_remove(JxEventContext *c,JxFixedAnimationBridge *b) {
    assert(c->call==fixed_player_dispatch && c->opaque==b && b->active==0);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
