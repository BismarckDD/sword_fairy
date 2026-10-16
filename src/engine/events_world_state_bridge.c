#include "event_engine.h"
#include <assert.h>
/* The guest's divide fault is latched until removal/reinstallation. The
 * host must check can_step before IRQ production and loop scheduling. Guest
 * requests after a fault report the latched flow without invoking devices.
 * This is a virtual CPU halt boundary, not a fabricated normal guest return. */
static uint32_t world_state_dispatch(void *opaque,uint16_t segment,
    uint16_t offset,const uint16_t *args,size_t count) {
    JxWorldStateBridge *b=opaque;
    if(b->flow!=JX_WORLD_UPDATE_RETURNED)return (uint32_t)b->flow;
    if((segment==0x2f28 && (offset==0xa332 || offset==0x8fd4 || offset==0x99b6 || offset==0x9b54)) ||
        (segment==0x1f2a && offset==0xdc10)) {
        assert(count==0);JxEventContext *c=b->base.context;
        if(segment==0x1f2a)b->flow=jx_world_npc_update(c);
        else if(offset==0xa332)b->flow=jx_world_tick_state(c);
        else if(offset==0x8fd4)b->flow=jx_world_player_move(c);
        else if(offset==0x99b6)b->flow=jx_world_idle_redraw(c);
        else b->flow=jx_world_background_cycle(c);
        if(b->flow!=JX_WORLD_UPDATE_RETURNED) {
            if(b->flow==JX_WORLD_PLAYER_DIVIDE_ERROR) {
                b->fault_segment=0x2f28;b->fault_offset=0x8fdf;
            } else if(b->flow==JX_WORLD_NPC_IDLE_DIVIDE_ERROR ||
                b->flow==JX_WORLD_NPC_ACTIVE_DIVIDE_ERROR) {
                b->fault_segment=0x1f2a;
                b->fault_offset=b->flow==JX_WORLD_NPC_IDLE_DIVIDE_ERROR?0xdc6a:0xdcd7;
            } else {
                b->fault_segment=0x2f28;
                b->fault_offset=b->flow==JX_WORLD_IDLE_DIVIDE_ERROR?0x99db:0x9b86;
            }
        }
        return (uint32_t)b->flow;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
void jx_world_state_bridge_install(JxEventContext *c,JxWorldStateBridge *b) {
    assert(c && b && c->call && c->call!=world_state_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->flow=JX_WORLD_UPDATE_RETURNED;b->fault_segment=0;b->fault_offset=0;
    c->call=world_state_dispatch;c->opaque=b;
}
void jx_world_state_bridge_remove(JxEventContext *c,JxWorldStateBridge *b) {
    assert(b->base.context==c && c->call==world_state_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
int jx_world_state_bridge_can_step(const JxWorldStateBridge *b) {
    return b->flow==JX_WORLD_UPDATE_RETURNED;
}
