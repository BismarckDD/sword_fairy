#include "event_engine.h"
#include <assert.h>
static uint32_t chapter_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *a,size_t n) {
    JxChapterBridge *b=opaque;
    if(segment==0x1f2a && offset==0x55f2) {
        assert(n==0);jx_chapter_text_load(b->base.context,b->work);return 0;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_chapter_bridge_install(JxEventContext *c,JxChapterBridge *b,JxFarPointer work) {
    assert(c && b && c->call && c->call!=chapter_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->work=work;c->call=chapter_dispatch;c->opaque=b;
}
void jx_chapter_bridge_remove(JxEventContext *c,JxChapterBridge *b) {
    assert(c->call==chapter_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
