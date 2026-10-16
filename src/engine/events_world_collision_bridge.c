#include "event_engine.h"
#include <assert.h>
/* Checked world roots use the actual scene collision implementation.
 * Faults are sticky and stop scheduling; return values on faults are flow
 * diagnostics, never a substitute for a normal guest AX/RETF. */
static uint32_t collision_dispatch(void *opaque,uint16_t segment,
        uint16_t offset,const uint16_t *args,size_t count) {
    JxWorldCollisionBridge *b=opaque;
    JxWorldStateBridge *s=&b->state;
    if(s->flow!=JX_WORLD_UPDATE_RETURNED)return (uint32_t)s->flow;
    JxEventContext *c=s->base.context;
    uint16_t result=0;
    if(segment==0x2f28 && offset==0x88de) {
        assert(count==0);s->flow=jx_party_check_triggers_world(c,b->scratch);
    } else if(segment==0x2f28 && (offset==0x87d6 || offset==0x4d12)) {
        assert(count==2);
        if(offset==0x87d6)s->flow=jx_player_cell_status_collision(c,args[0],args[1],b->scratch,&result);
        else s->flow=jx_scene_collision_layers(c,args[0],args[1],b->scratch);
    } else if(segment==0x1f2a && (offset==0xd076 || offset==0xda5a)) {
        assert(count==1);
        s->flow=offset==0xd076?jx_npc_idle_update_cells(c,args[0],b->scratch):jx_npc_active_update(c,args[0]);
    } else if((segment==0x2f28 && (offset==0xa332 || offset==0x8fd4 || offset==0x99b6 || offset==0x9b54)) ||
              (segment==0x1f2a && offset==0xdc10)) {
        assert(count==0);
        if(segment==0x1f2a)s->flow=jx_world_npc_update_cells(c,b->scratch);
        else if(offset==0xa332)s->flow=jx_world_tick_collision(c,b->scratch);
        else if(offset==0x8fd4)s->flow=jx_world_player_move_collision(c,b->scratch);
        else if(offset==0x99b6)s->flow=jx_world_idle_redraw_cells(c,b->scratch);
        else s->flow=jx_world_background_cycle_cells(c,b->scratch);
    } else return s->base.fallback(s->base.fallback_opaque,segment,offset,args,count);
    if(s->flow!=JX_WORLD_UPDATE_RETURNED) {
        static const uint16_t segments[]={0,0x2f28,0x2f28,0x1f2a,0x1f2a,0x2f28,0x2f28,0x2f28,0x1f2a,0x1f2a,0x2f28,0x2f28};
        static const uint16_t offsets[]={0,0x99db,0x9b86,0xdc6a,0xdcd7,0x8fdf,0x4e3b,0x4e89,0xd0c3,0xdaa1,0x4b3c,0x4b8a};
        assert((unsigned)s->flow<sizeof offsets/sizeof offsets[0]);
        s->fault_segment=segments[s->flow];s->fault_offset=offsets[s->flow];
        return (uint32_t)s->flow;
    }
    return result;
}
void jx_world_collision_bridge_install(JxEventContext *c,JxWorldCollisionBridge *b,JxFarPointer scratch) {
    assert(c && b && c->call && c->call!=collision_dispatch);
    b->state.base.context=c;b->state.base.fallback=c->call;b->state.base.fallback_opaque=c->opaque;
    b->state.flow=JX_WORLD_UPDATE_RETURNED;b->state.fault_segment=0;b->state.fault_offset=0;
    b->scratch=scratch;c->call=collision_dispatch;c->opaque=b;
}
void jx_world_collision_bridge_remove(JxEventContext *c,JxWorldCollisionBridge *b) {
    assert(b->state.base.context==c && c->call==collision_dispatch && c->opaque==b);
    c->call=b->state.base.fallback;c->opaque=b->state.base.fallback_opaque;
}
