#include <assert.h>
#include "event_engine.h"
static uint32_t trigger_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    JxEngineBridge *b=(JxEngineBridge *)opaque;
    if(segment==0x2f28 && offset==0x88de) {assert(count==0);jx_party_check_triggers(b->context);return 0;}
    if(segment==0x1f2a && offset==0xc24e) {assert(count==0);return jx_event_clear_state(b->context);}
    if(segment==0x2f28 && offset==0x86c4) {jx_event_empty(b->context);return 0;}
    return b->fallback(b->fallback_opaque,segment,offset,args,count);
}
void jx_trigger_bridge_install(JxEventContext *c,JxEngineBridge *b) {
    assert(c && b && c->call && c->call!=trigger_dispatch);
    b->context=c;b->fallback=c->call;b->fallback_opaque=c->opaque;c->call=trigger_dispatch;c->opaque=b;
}
void jx_trigger_bridge_remove(JxEventContext *c,JxEngineBridge *b) {
    assert(c->opaque==b && c->call==trigger_dispatch);c->call=b->fallback;c->opaque=b->fallback_opaque;
}
