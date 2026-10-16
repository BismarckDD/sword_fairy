#include "event_engine.h"
#include <assert.h>
static uint32_t game_control_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *args,size_t count) {
    JxGameControlBridge *b=opaque;uint16_t bit=0;
    if(segment==0x1f2a) {
        if(offset==0xb1fa)bit=1;
        else if(offset==0x6b38)bit=2;
        else if(offset==0x5f94)bit=4;
    }
    if(!bit)return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
    assert(!(b->active&bit));
    assert(count==0 || (bit==1 && count==5));
    b->active|=bit;uint16_t result=0;
    if(bit==1)result=jx_game_entry(b->base.context);
    else if(bit==2)jx_game_initialize(b->base.context);
    else jx_graphics_initialize(b->base.context);
    b->active&=(uint16_t)~bit;return result;
}
void jx_game_control_bridge_install(JxEventContext *c,JxGameControlBridge *b) {
    assert(c && b && c->call && c->call!=game_control_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->active=0;c->call=game_control_dispatch;c->opaque=b;
}
void jx_game_control_bridge_remove(JxEventContext *c,JxGameControlBridge *b) {
    assert(c->call==game_control_dispatch && c->opaque==b && !b->active);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
