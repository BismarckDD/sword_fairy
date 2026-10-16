#include "event_engine.h"
static void battle_draw_pages(JxEventContext *c,uint16_t page,uint16_t side) {
    (void)JX_CALL(c,0x1503,0x007d,0,page,jx_word(c,0x9332));
    (void)JX_CALL(c,0x1503,0x007d,1,(uint16_t)(page+1),jx_word(c,0x9332));
    uint16_t x=side==0?40:(side==1?jx_word(c,0x292c):100);
    uint16_t y=side==0?10:(side==1?jx_word(c,0x292e):40);
    (void)JX_CALL(c,0x1832,0x1a28,x,y,220,148,jx_word(c,0x5858),jx_word(c,0x585a),255);
}
/* 1F2A:2C9C. Values other than 0 or 1 intentionally use the third actor. */
void jx_battle_draw_animation_frame(JxEventContext *c,uint16_t side,uint16_t frame) {
    uint16_t first=side==0?87:(side==1?103:119);
    battle_draw_pages(c,(uint16_t)(first+frame*2),side);
}
/* 1F2A:2A9E. Death is a signed HP test; low HP uses an unsigned quarter
 * of maximum HP, without treating a negative maximum as zero. */
void jx_battle_draw_actor_pose(JxEventContext *c,uint16_t side) {
    if(side>2)return;
    if(side==0){(void)JX_CALL(c,0x1f2a,0x2c9c,0,0);return;}
    uint16_t actor=jx_word(c,side==1?0x2936:0x2938);
    uint16_t hp=jx_word(c,(uint16_t)(0xd04e + actor*68+0x10));
    uint16_t page;
    if(jx_signed_word(hp)<=0)page=side==1?145:149;
    else {
        uint16_t maximum=jx_word(c,(uint16_t)(0xd04e + actor*68+0x14));
        if(hp>maximum/4){(void)JX_CALL(c,0x1f2a,0x2c9c,side,0);return;}
        page=side==1?143:147;
    }
    battle_draw_pages(c,page,side);
}
/* 1F2A:3D1C. Private 62-byte frame: number[12], filename[50]. */
void jx_battle_cache_actor_animation(JxEventContext *c,uint16_t side,uint16_t number,JxFarPointer work) {
    if(side>2 || jx_signed_word(number)<0)return;
    JxFarPointer filename={(uint16_t)(work.offset+12),work.segment};
    JxFarPointer prefix={side==0?0x00f6:0x00ea,c->data_segment};
    (void)jx_string_copy(c,filename,prefix);
    (void)JX_CALL(c,0x1000,0x2a8c,work.offset,work.segment,
                  side==0?0x0106:0x0102,c->data_segment,
                  side==0?number:(uint16_t)(number-100));
    for(uint16_t i=0;i<2;i++)
        if(jx_far_byte(c,work,i)==0x20)jx_far_set_byte(c,work,i,0x30);
    jx_far_set_byte(c,work,3,0);
    (void)jx_string_append(c,filename,work);
    (void)jx_string_append(c,filename,(JxFarPointer){0x010a,c->data_segment});
    (void)JX_CALL(c,0x1f2a,0x288a,filename.offset,filename.segment,
                  side==0?87:(side==1?103:119),side==0?1:14,side==0?1:27,220,148);
}
