#include <assert.h>
#include "event_engine.h"
static uint32_t choice_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxChoiceBridge *b=(JxChoiceBridge *)opaque;
    if(segment==0x1f2a && (offset==0x7baa || offset==0x7e0e)) {
        assert(!b->active);b->active=1;uint16_t result;
        if(offset==0x7baa) {
            assert(n==6);result=jx_two_choice_menu(b->base.context,a[0],a[1],(JxFarPointer){a[2],a[3]},(JxFarPointer){a[4],a[5]},b->event);
        } else {
            assert(n==8);result=jx_choice_menu(b->base.context,a[0],a[1],(JxFarPointer){a[2],a[3]},(JxFarPointer){a[4],a[5]},(JxFarPointer){a[6],a[7]},b->event);
        }
        b->active=0;return result;
    }
    if(segment==0x1f2a && offset==0xc1f2) {assert(n==2);return (uint16_t)jx_input_dequeue(b->base.context,a[0],a[1]);}
    if(segment==0x1f2a && offset==0xc24e) {assert(n==0);return jx_event_clear_state(b->base.context);}
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_choice_bridge_install(JxEventContext *c,JxChoiceBridge *b,JxFarPointer event) {
    assert(c && b && c->call && c->call!=choice_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->event=event;b->active=0;
    c->call=choice_dispatch;c->opaque=b;
}
void jx_choice_bridge_remove(JxEventContext *c,JxChoiceBridge *b) {
    assert(c->call==choice_dispatch && c->opaque==b && !b->active);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
