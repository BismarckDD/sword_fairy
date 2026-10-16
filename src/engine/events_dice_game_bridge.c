#include <assert.h>
#include "event_engine.h"
static uint32_t dice_game_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxDiceGameBridge *b=(JxDiceGameBridge *)opaque;
    if(segment==0x1f2a && offset==0xb6a0) {
        assert(n==3 && !b->active);b->active=1;
        uint16_t result=jx_dice_game(b->base.context,a[0],a[1],a[2],b->frame);b->active=0;return result;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
/*248 frame bytes must not alias the helper bridges' private workspaces. */
void jx_dice_game_bridge_install(JxEventContext *c,JxDiceGameBridge *b,JxFarPointer frame) {
    assert(c && b && c->call && c->call!=dice_game_dispatch && frame.offset<=65288);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->frame=frame;b->active=0;
    c->call=dice_game_dispatch;c->opaque=b;
}
void jx_dice_game_bridge_remove(JxEventContext *c,JxDiceGameBridge *b) {
    assert(c->call==dice_game_dispatch && c->opaque==b && !b->active);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
