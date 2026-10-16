#include "event_engine.h"
#include <assert.h>
static uint32_t menu_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *a,size_t n) {
    JxMenuBridge *b=opaque;
    if(segment==0x1f2a && offset==0x9dc2) {
        assert(n==0);jx_save_game_menu(b->base.context,b->load_work);return 0;
    }
    if(segment==0x1f2a && offset==0xa2f0) {
        assert(n==0);return jx_load_game_menu(b->base.context,b->load_work);
    }
    if(segment==0x1f2a && offset==0x7baa) {
        assert(n==6);
        return jx_two_choice_menu(b->base.context,a[0],a[1],
            (JxFarPointer){a[2],a[3]},(JxFarPointer){a[4],a[5]},b->event_work);
    }
    if(segment==0x1f2a && offset==0x7e0e) {
        assert(n==8);
        return jx_choice_menu(b->base.context,a[0],a[1],
            (JxFarPointer){a[2],a[3]},(JxFarPointer){a[4],a[5]},
            (JxFarPointer){a[6],a[7]},b->event_work);
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_menu_bridge_install(JxEventContext *c,JxMenuBridge *b,JxFarPointer event_work,JxFarPointer load_work) {
    assert(c && b && c->call && c->call!=menu_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->event_work=event_work;b->load_work=load_work;c->call=menu_dispatch;c->opaque=b;
}
void jx_menu_bridge_remove(JxEventContext *c,JxMenuBridge *b) {
    assert(c->call==menu_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
