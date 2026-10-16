#include <assert.h>
#include "event_engine.h"
static JxFarPointer battle_work(JxBattleBridge *b,uint16_t offset) {
    JxFarPointer p={(uint16_t)(b->work.offset+offset),b->work.segment};return p;
}
static uint16_t battle_feature(uint16_t offset) {
    switch(offset) {
    case 0x4934:case 0x3e88:case 0x2fc6:case 0x71de:
    case 0x3eee:case 0x3eb2:case 0x3f12:case 0x4288:return JX_BATTLE_CONTROL;
    case 0x80fa:case 0x9952:case 0x9712:case 0x9182:case 0x8f78:case 0x864a:return JX_BATTLE_MENUS;
    case 0x46e6:case 0x4830:return JX_BATTLE_IMAGES;
    case 0x2c9c:case 0x2a9e:case 0x2e10:return JX_BATTLE_DRAW;
    case 0x3d1c:case 0x288a:return JX_BATTLE_CACHE;
    case 0x44ac:case 0x43b2:case 0x747c:return JX_BATTLE_SETTLEMENT;
    case 0x8bd6:case 0x8f28:return JX_BATTLE_INVENTORY;
    case 0xc1f2:case 0xc24e:return JX_BATTLE_INPUT;
    default:return 0;
    }
}
static uint32_t battle_invoke(JxBattleBridge *b,uint16_t o,const uint16_t *a,size_t n) {
    JxEventContext *c=b->base.context;
    switch(o) {
    case 0x4934:assert(n==3);return jx_battle_run(c,a[0],a[1],a[2],battle_work(b,0));
    case 0x3e88:assert(n==4);jx_battle_turn_dispatch(c);return 0;
    case 0x2fc6:assert(n==0);jx_battle_run_turn(c,battle_work(b,20));return 0;
    case 0x71de:assert(n==1);return jx_chance_percent(c,a[0]);
    case 0x3eee:assert(n==0);jx_battle_choose_attack(c);return 0;
    case 0x3eb2:assert(n==0);jx_battle_open_actor_menu(c);return 0;
    case 0x3f12:assert(n==0);jx_battle_choose_item(c);return 0;
    case 0x4288:assert(n==0);jx_battle_choose_skill(c);return 0;
    case 0x80fa:assert(n==5);return jx_action_menu(c,a[0],a[1],a[2],(JxFarPointer){a[3],a[4]},battle_work(b,72));
    case 0x9952:assert(n==1);return jx_skill_menu(c,a[0],battle_work(b,106));
    case 0x9712:assert(n==3);jx_skill_menu_render(c,a[0],a[1],a[2],battle_work(b,140));return 0;
    case 0x9182:assert(n==0);return jx_item_menu(c,battle_work(b,168));
    case 0x8f78:assert(n==3);jx_item_menu_render(c,a[0],a[1],a[2],battle_work(b,204));return 0;
    case 0x864a:assert(n==1);jx_actor_menu_show(c,a[0],battle_work(b,234));return 0;
    case 0x46e6:assert(n==1);jx_battle_load_background(c,a[0],battle_work(b,294));return 0;
    case 0x4830:assert(n==2);jx_battle_load_actor_image(c,a[0],a[1],battle_work(b,294));return 0;
    case 0x2c9c:assert(n==2);jx_battle_draw_animation_frame(c,a[0],a[1]);return 0;
    case 0x2a9e:assert(n==1);jx_battle_draw_actor_pose(c,a[0]);return 0;
    case 0x2e10:assert(n==2);jx_battle_show_damage(c,a[0],a[1],battle_work(b,594));return 0;
    case 0x3d1c:assert(n==2);jx_battle_cache_actor_animation(c,a[0],a[1],battle_work(b,368));return 0;
    case 0x288a:assert(n==7);return jx_battle_cache_animation(c,(JxFarPointer){a[0],a[1]},a[2],a[3],a[4],a[5],a[6]);
    case 0x44ac:assert(n==0);jx_battle_award_experience(c,battle_work(b,430));return 0;
    case 0x43b2:assert(n==2);jx_actor_level_up(c,a[0],a[1],battle_work(b,528));return 0;
    case 0x747c:assert(n==1);jx_actor_restore_current(c,a[0]);return 0;
    case 0x8bd6:assert(n==0);jx_inventory_compact(c);return 0;
    case 0x8f28:assert(n==2);jx_inventory_remove(c,a[0],a[1]);return 0;
    case 0xc1f2:assert(n==2);return (uint16_t)jx_input_dequeue(c,a[0],a[1]);
    case 0xc24e:assert(n==0);jx_input_clear(c);return 0;
    default:assert(0);return 0;
    }
}
static uint32_t battle_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxBattleBridge *b=opaque;
    uint16_t feature=segment==0x1f2a?battle_feature(offset):0;
    if(!(feature&b->features))return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
    /* Fixed frame slots must not be recursively reused. Distinct callers
     * may nest: controller -> menu -> action -> submenu -> renderer. */
    static const uint16_t entries[]={0x4934,0x3e88,0x2fc6,0x71de,0x3eee,0x3eb2,0x3f12,0x4288,0x80fa,0x9952,0x9712,0x9182,0x8f78,0x864a,0x46e6,0x4830,0x2c9c,0x2a9e,0x2e10,0x3d1c,0x288a,0x44ac,0x43b2,0x747c,0x8bd6,0x8f28,0xc1f2,0xc24e};
    unsigned entry=0;while(entries[entry]!=offset)entry++;
    uint32_t mask=UINT32_C(1)<<entry;
    if(offset==0x46e6 || offset==0x4830)mask=(UINT32_C(1)<<14)|(UINT32_C(1)<<15);
    assert(!(b->active&mask));b->active|=mask;
    uint32_t result=battle_invoke(b,offset,a,n);b->active&=~mask;return result;
}
void jx_battle_bridge_install(JxEventContext *c,JxBattleBridge *b,JxFarPointer work,uint16_t features) {
    assert(c && b && c->call && c->call!=battle_dispatch);
    assert(work.offset<=65536-JX_BATTLE_WORK_BYTES && !(features&~JX_BATTLE_ALL));
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->work=work;b->features=features;b->active=0;c->call=battle_dispatch;c->opaque=b;
}
void jx_battle_bridge_remove(JxEventContext *c,JxBattleBridge *b) {
    assert(c->call==battle_dispatch && c->opaque==b && !b->active);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
