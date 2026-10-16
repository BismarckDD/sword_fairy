#include <assert.h>
#include "event_engine.h"
/* Movement dispatch remains composable with video/EMS/runtime bridges. */
static uint32_t movement_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxMovementBridge *b=(JxMovementBridge *)opaque;JxEventContext *c=b->base.context;
    if(segment==0x2f28) switch(offset) {
    case 0x95aa:assert(n==0);jx_scene_run_trajectories(c);return 0;
    case 0x86ce:assert(n==2);return jx_movement_classify(c,a[0],a[1]);
    case 0x944c:assert(n==3);jx_party_run_moves(c,(JxFarPointer){a[0],a[1]},a[2]);return 0;
    case 0x8c6e:assert(n==0);jx_party_advance_frames(c);return 0;
    case 0x8d50:assert(n==0);jx_party_shift_trail(c);return 0;
    case 0x5166:assert(n==0);jx_party_redraw_trail(c);return 0;
    case 0x8dd6:assert(n==2);jx_party_move_cycle(c,a[0],a[1]);return 0;
    case 0x5346:assert(n==0);jx_party_redraw_page(c);return 0;
    case 0x53b0:assert(n==1);jx_party_scroll_page(c,a[0]);return 0;
    default:break;
    }
    if(segment==0x1f2a) switch(offset) {
    case 0xcf6a:assert(n==3);jx_npc_walk_to(c,a[0],a[1],a[2],b->path);return 0;
    case 0xca66:assert(n==1);jx_npc_redraw_background(c,(int16_t)jx_signed_word(a[0]));return 0;
    case 0xcd1c:assert(n==4);jx_npc_run_moves(c,a[0],(JxFarPointer){a[1],a[2]},a[3]);return 0;
    case 0xcb08:assert(n==4);return jx_choose_step(c,a[0],a[1],a[2],a[3]);
    case 0xcc24:assert(n==2);jx_party_walk_to(c,a[0],a[1],b->path);return 0;
    default:break;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_movement_bridge_install(JxEventContext *c,JxMovementBridge *b,JxFarPointer path) {
    assert(c && b && c->call && c->call!=movement_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->path=path;
    c->call=movement_dispatch;c->opaque=b;
}
void jx_movement_bridge_remove(JxEventContext *c,JxMovementBridge *b) {
    assert(c->call==movement_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
