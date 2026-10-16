#include "event_engine.h"
#include <assert.h>

static uint32_t startup_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *args,size_t count) {
    JxVideoStartupBridge *b=opaque;
    if(segment==0x1832 && offset==0x17ae) {
        assert(count==0);
        jx_video_enter_mode_x(&b->bus,b->bios_mode,b->bios_opaque);return 0;
    }
    if(offset==0x000a && (segment==0x1502 || segment==0x150e)) {
        assert(count==0);
        if(segment==0x1502)jx_video_blank_screen(&b->bus);
        else jx_video_unblank_screen(&b->bus);
        return 0;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
void jx_video_startup_bridge_install(JxEventContext *c,JxVideoStartupBridge *b,
    const JxVideoBus *bus,void (*bios_mode)(void *,uint16_t),void *opaque) {
    assert(c && b && bus && c->call && c->call!=startup_dispatch);
    assert(bus->out && bus->in_byte && bus->write_byte && bios_mode);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->bus=*bus;b->bios_mode=bios_mode;b->bios_opaque=opaque;
    c->call=startup_dispatch;c->opaque=b;
}
void jx_video_startup_bridge_remove(JxEventContext *c,JxVideoStartupBridge *b) {
    assert(c->opaque==b && c->call==startup_dispatch);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
