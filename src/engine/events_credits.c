#include "event_engine.h"

static void credits_map(JxEventContext *c,uint16_t first) {
    for (uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
/* 2F28:7C44. ZZQ.IMG holds eighteen contiguous 320x200 images. */
void jx_credits_cache(JxEventContext *c) {
    uint32_t file=JX_CALL(c,0x1000,0x738,0xdec,c->data_segment,0xde9,c->data_segment);
    if (!file) return;
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),16,0,0);
    for (uint16_t image=0;image<18;image++) {
        credits_map(c,(uint16_t)(87+4*image));
        (void)JX_CALL(c,0x1000,0x754,jx_word(c,0x5858),jx_word(c,0x585a),1,64000,(uint16_t)file,(uint16_t)(file>>16));
    }
    (void)JX_CALL(c,0x1000,0x754,0x2eb4,c->data_segment,3,256,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
}
/* 2F28:7D4E. Signed row division and offset-only source addition are
 * intentional. The second destination addition uses huge-pointer carry. */
void jx_credits_scroll_frame(JxEventContext *c,uint16_t position) {
    int16_t image=(int16_t)position/200,row=(int16_t)position%200;
    credits_map(c,(uint16_t)(87+4*image));
    (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x2eb0),jx_word(c,0x2eb2),
        (uint16_t)(jx_word(c,0x5858)+320*row),jx_word(c,0x585a),(uint16_t)(320*(200-row)));
    if (row>0) {
        credits_map(c,(uint16_t)(91+4*image));
        uint32_t offset=(uint32_t)jx_word(c,0x2eb0)+(uint16_t)(320*(200-row));
        (void)JX_CALL(c,0x1000,0x3146,(uint16_t)offset,
            (uint16_t)(jx_word(c,0x2eb2)+((offset>>16)<<12)),
            jx_word(c,0x5858),jx_word(c,0x585a),(uint16_t)(320*row));
    }
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
}
/* 2F28:7EC8. Timer events scroll; type-zero input exits. The original
 * timer counter is only used modulo one, so it has no observable effect. */
void jx_credits_run(JxEventContext *c,JxFarPointer event_work) {
    uint16_t position=0;
    (void)JX_CALL(c,0x2f28,0x6d18,0xdf4,c->data_segment);
    (void)JX_CALL0(c,0x1f2a,0x7272);
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    (void)JX_CALL0(c,0x2f28,0x7c44);
    (void)JX_CALL(c,0x1832,0x1810,0);
    (void)JX_CALL(c,0x2f28,0x7d4e,0);
    (void)JX_CALL(c,0x1832,0x1836,0);
    (void)JX_CALL(c,0x1f2a,0x720e,1000);
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    if (jx_word(c,0x31b4)!=0) {
        uint16_t seconds=jx_far_word(c,jx_word(c,0x23aa),0x538);
        (void)JX_CALL(c,0x1832,0x224c,(uint8_t)(jx_ds(c)[0x932c]+20),2,seconds);
    }
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for (;;) {
        while ((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event_work.offset,event_work.segment)!=0) {}
        uint16_t kind=jx_far_word(c,event_work.segment,event_work.offset);
        if (kind==0) {
            (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
            return;
        }
        if (kind==8) {
            (void)JX_CALL(c,0x1832,0x1810,position%2);
            (void)JX_CALL(c,0x2f28,0x7d4e,position);
            (void)JX_CALL(c,0x1832,0x1836,position%2);
            if (position<3399) position++;
        }
    }
}
