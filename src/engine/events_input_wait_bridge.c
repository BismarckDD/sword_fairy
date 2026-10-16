#include <assert.h>
#include "event_engine.h"
static uint32_t input_wait_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxInputWaitBridge *b=opaque;JxEventContext *c=b->base.context;
    if(segment!=0x1f2a || (offset!=0x2dbc && offset!=0x72b2 && offset!=0x7272 && offset!=0x731a))
        return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
    assert(b->active==0);b->active=1;
    switch(offset) {
    case 0x2dbc:assert(n==1);jx_delay(c,a[0],b->poll,b->poll_opaque);break;
    case 0x72b2:assert(n==1);jx_wait_key_timed(c,a[0],b->poll,b->poll_opaque);break;
    case 0x7272:assert(n==0);jx_wait_key(c,b->poll,b->poll_opaque);break;
    case 0x731a:assert(n==0);jx_wait_key_music(c,b->poll,b->poll_opaque);break;
    default:assert(0);break;
    }
    b->active=0;return 0;
}
/* The host supplies event polling and scheduling. This adapter does not
 * generate timer/keyboard events or substitute a timeout for missing input. */
void jx_input_wait_bridge_install(JxEventContext *c,JxInputWaitBridge *b,JxInputPoll poll,void *opaque) {
    assert(c && b && c->call && c->call!=input_wait_dispatch && poll);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->poll=poll;b->poll_opaque=opaque;b->active=0;c->call=input_wait_dispatch;c->opaque=b;
}
void jx_input_wait_bridge_remove(JxEventContext *c,JxInputWaitBridge *b) {
    assert(c->call==input_wait_dispatch && c->opaque==b && b->active==0);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
