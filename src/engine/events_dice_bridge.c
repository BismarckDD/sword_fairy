#include <assert.h>
#include "event_engine.h"
static uint32_t dice_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxDiceBridge *b=(JxDiceBridge *)opaque;JxEventContext *c=b->base.context;
    if(segment==0x1f2a) {
        if(offset==0xb4ba) {
            assert(n==6);jx_dice_reorder(c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},(JxFarPointer){a[4],a[5]});return 0;
        }
        if(offset==0xb522 || offset==0xb21a) {
            assert(!b->active);b->active=1;uint16_t result=0;
            if(offset==0xb522) {assert(n==3);result=jx_dice_choose_wager(c,a[0],a[1],a[2],b->work);}
            else {assert(n==6);jx_dice_play_animation(c,(JxFarPointer){a[0],a[1]},a[2],a[3],a[4],a[5],(JxFarPointer){(uint16_t)(b->work.offset+20),b->work.segment});}
            b->active=0;return result;
        }
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
/*28 disjoint private bytes: wager20 then FLM header8. */
void jx_dice_bridge_install(JxEventContext *c,JxDiceBridge *b,JxFarPointer work) {
    assert(c && b && c->call && c->call!=dice_dispatch && work.offset<=65508);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->work=work;b->active=0;
    c->call=dice_dispatch;c->opaque=b;
}
void jx_dice_bridge_remove(JxEventContext *c,JxDiceBridge *b) {
    assert(c->call==dice_dispatch && c->opaque==b && !b->active);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
