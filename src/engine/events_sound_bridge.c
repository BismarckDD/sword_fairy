#include "event_engine.h"
#include <assert.h>

static uint32_t sound_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *a,size_t n) {
    JxSoundBridge *b=opaque;JxEventContext *c=b->base.context;
    if (segment==0x1f2a) switch (offset) {
    case 0x155c:assert(n==1);return jx_sound_play(c,a[0]);
    case 0x2820:assert(n==1);return jx_sound_dsp_reset(c,a[0]);
    case 0x27e4:assert(n==2);jx_sound_dsp_write(c,a[0],a[1]);return 0;
    case 0x16b0:
        assert(n==5);jx_sound_dma_setup(c,(uint32_t)a[0]|((uint32_t)a[1]<<16),
            (JxFarPointer){a[2],a[3]},a[4],b->dma_stack);return 0;
    case 0x18f2:
        assert(n==4);jx_sound_dsp_start(c,a[0],(JxFarPointer){a[1],a[2]},a[3],b->dsp_stack);return 0;
    default:break;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
/* Normal configurations overwrite these scratch seeds. Explicit seeds
 * retain unsupported configuration behavior of the original private stack. */
void jx_sound_bridge_install(JxEventContext *c,JxSoundBridge *b,
    JxSoundDmaPorts dma_stack,JxSoundDspScratch dsp_stack) {
    assert(c && b && c->call && c->call!=sound_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->dma_stack=dma_stack;b->dsp_stack=dsp_stack;c->call=sound_dispatch;c->opaque=b;
}
void jx_sound_bridge_remove(JxEventContext *c,JxSoundBridge *b) {
    assert(c->call==sound_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
