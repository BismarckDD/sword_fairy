#include "event_engine.h"
static uint16_t experience_field(JxEventContext *c,uint16_t global,uint16_t field) {
    return (uint16_t)(0xd04e + jx_word(c,global)*68+field);
}
static uint16_t level_factor(JxEventContext *c,uint16_t global) {
    return (uint16_t)(jx_signed_word(jx_word(c,experience_field(c,global,0x22)))/10+1);
}
static uint16_t level_threshold(JxEventContext *c,uint16_t global,uint16_t factor) {
    uint16_t level=jx_word(c,experience_field(c,global,0x22));
    return (uint16_t)((uint32_t)(uint16_t)(level*40)*factor+50);
}
static void battle_award_levels(JxEventContext *c,uint16_t global,uint16_t factor,uint16_t message_y) {
    for(;;) {
        uint16_t threshold=level_threshold(c,global,factor);
        uint16_t field=experience_field(c,global,0x1e);
        if(jx_word(c,field)<threshold)return;
        threshold=level_threshold(c,global,factor);
        field=experience_field(c,global,0x1e);
        uint16_t remaining=(uint16_t)(jx_word(c,field)-threshold);
        jx_set_word(c,field,remaining);
        jx_set_word(c,experience_field(c,global,0x1e),remaining);
        (void)JX_CALL(c,0x1f2a,0x43b2,jx_word(c,global),message_y);
        factor=level_factor(c,global);
    }
}
/* 1F2A:44AC. Word-wrapped round factor, 32-bit product/signed quotient,
 * 16-bit awarded experience and unsigned level thresholds. Work98 bytes. */
void jx_battle_award_experience(JxEventContext *c,JxFarPointer work) {
    uint16_t round=(uint16_t)(jx_word(c,0x2932)+9);
    uint16_t reward=jx_word(c,(uint16_t)(0x499e + jx_word(c,0x2934)*30+0x1c));
    uint32_t product=jx_multiply_u32(reward,(uint32_t)(int32_t)jx_signed_word(round)),quotient=0;
    (void)jx_divide_signed32(product,10,&quotient);
    uint16_t amount=(uint16_t)quotient;
    JxFarPointer number={(uint16_t)(work.offset+6),work.segment};
    JxFarPointer message={(uint16_t)(work.offset+32),work.segment};
    (void)jx_string_copy(c,message,(JxFarPointer){0x0134,c->data_segment});
    (void)JX_CALL(c,0x1000,0x2a8c,number.offset,number.segment,0x013d,c->data_segment,amount);
    (void)jx_string_append(c,message,number);
    (void)jx_string_append(c,message,(JxFarPointer){0x0141,c->data_segment});
    (void)JX_CALL(c,0x1f2a,0x0728,100,96,message.offset,message.segment);
    uint16_t field=experience_field(c,0x2936,0x1e);
    jx_set_word(c,field,(uint16_t)(jx_word(c,field)+amount));
    battle_award_levels(c,0x2936,level_factor(c,0x2936),76);
    if(jx_word(c,0x2938)==jx_word(c,0x2936))return;
    uint16_t factor=level_factor(c,0x2938);
    field=experience_field(c,0x2938,0x1e);
    jx_set_word(c,field,(uint16_t)(jx_word(c,field)+amount));
    battle_award_levels(c,0x2938,factor,116);
}
