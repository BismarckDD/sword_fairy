#include "event_engine.h"

/* Keep offset overflow as 1000h paragraphs; do not normalize the pointer. */
static JxFarPointer portrait_buffer(JxEventContext *c,uint16_t displacement) {
    uint32_t offset=(uint32_t)jx_word(c,0x2eb0)+displacement;
    JxFarPointer result={(uint16_t)offset,
        (uint16_t)(jx_word(c,0x2eb2)+((offset>>16)<<12))};
    return result;
}
static void portrait_transition(JxEventContext *c,uint16_t x,uint16_t portrait) {
    JxFarPointer old_image=portrait_buffer(c,32000);
    (void)JX_CALL(c,0x1832,0x1e88,x,10,60,70,old_image.offset,old_image.segment);
    int16_t other=(int16_t)(uint16_t)(jx_word(c,0x3e54)+1);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(other%2));
    (void)JX_CALL(c,0x1f2a,0x752e,x,10,portrait);
    JxFarPointer new_image=portrait_buffer(c,48000);
    (void)JX_CALL(c,0x1832,0x1e88,x,10,60,70,new_image.offset,new_image.segment);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)((int16_t)jx_word(c,0x3e54)%2));
    old_image=portrait_buffer(c,32000);new_image=portrait_buffer(c,48000);
    (void)JX_CALL(c,0x17d2,0x1d4,x,10,60,70,
        old_image.offset,old_image.segment,new_image.offset,new_image.segment,
        jx_word(c,0x2eb0),jx_word(c,0x2eb2),0x61de,c->data_segment);
}
/* 2F28:276E: reveal portrait 178 at left, then portrait 250 at right. */
void jx_scene_portrait_pair(JxEventContext *c) {
    portrait_transition(c,20,178);
    portrait_transition(c,240,250);
}
