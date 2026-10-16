#include "event_engine.h"
#include <assert.h>
/* Serial host world-update dispatch. Nested tick -> ranges -> rotation is
 * ordinary engine nesting; unknown movement/scene/device calls remain real
 * dependencies forwarded to the installed host, never silently skipped. */
static uint32_t world_update_dispatch(void *opaque,uint16_t segment,
    uint16_t offset,const uint16_t *args,size_t count) {
    JxEngineBridge *b=opaque;JxEventContext *c=b->context;
    if(segment==0x1f2a && offset==0x71de) {
        assert(count==1);return jx_chance_percent(c,args[0]);
    }
    if(segment==0x2f28) {
        switch(offset) {
        case 0xa332:assert(count==0);jx_world_tick(c);return 0;
        case 0x9a20:assert(count==3);jx_palette_rotate_range(c,args[0],args[1],args[2]);return 0;
        case 0x9b04:assert(count==0);jx_world_palette_ranges(c);return 0;
        case 0xa1fa:assert(count==0);jx_world_periodic_sounds(c);return 0;
        case 0xa0fc:assert(count==0);jx_world_cd_update(c);return 0;
        case 0xa2e6:assert(count==0);jx_world_check_disc(c);return 0;
        default:break;
        }
    }
    return b->fallback(b->fallback_opaque,segment,offset,args,count);
}
void jx_world_update_bridge_install(JxEventContext *c,JxEngineBridge *b) {
    assert(c && b && c->call && c->call!=world_update_dispatch);
    b->context=c;b->fallback=c->call;b->fallback_opaque=c->opaque;
    c->call=world_update_dispatch;c->opaque=b;
}
void jx_world_update_bridge_remove(JxEventContext *c,JxEngineBridge *b) {
    assert(b->context==c && c->call==world_update_dispatch && c->opaque==b);
    c->call=b->fallback;c->opaque=b->fallback_opaque;
}
