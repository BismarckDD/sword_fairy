#include "event_engine.h"
static JxFarPointer erase_work(JxEventContext *c,uint16_t delta) {
    unsigned sum=(unsigned)jx_word(c,0x2eb0)+delta;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+(sum>>16)*0x1000u)};
}
/* 2F28:7A1C. Animate removal only if every occupied NPC tile is inside
 * the 20 by 12 camera rectangle. Dimensions and position are captured
 * before callbacks; page, scene and scratch globals remain live. */
void jx_npc_erase_transition(JxEventContext *c,uint16_t npc) {
    uint16_t record=(uint16_t)(0x889c+npc*0x56u);
    uint16_t shape=(uint16_t)(0x31ce + npc*0x12u);
    uint16_t x=jx_word(c,(uint16_t)(record+0x40));
    uint16_t y=jx_word(c,(uint16_t)(record+0x42));
    uint16_t width=jx_word(c,shape),height=jx_word(c,(uint16_t)(shape+2));
    uint16_t top=(uint16_t)(y-height+1);
    for(uint16_t row=0;jx_signed_word(row)<jx_signed_word(height);row++) {
        for(uint16_t column=0;jx_signed_word(column)<jx_signed_word(width);column++) {
            uint16_t local_x,local_y;
            if(jx_rectangle_local_point(&local_x,&local_y,(uint16_t)(x+column),(uint16_t)(top+row),
                    jx_word(c,0x8896),jx_word(c,0x8898),20,12)) return;
        }
    }
    x=(uint16_t)((x-jx_word(c,0x8896))*16u);
    y=(uint16_t)((top-jx_word(c,0x8898))*16u);
    uint16_t pixels_w=(uint16_t)(width*16u),pixels_h=(uint16_t)(height*16u);
    JxFarPointer before=erase_work(c,0x7d00);
    (void)JX_CALL(c,0x1832,0x1e88,x,y,pixels_w,pixels_h,before.offset,before.segment);
    uint16_t page=(uint16_t)(jx_word(c,0x3e54)+1);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(page)%2));
    jx_ds(c)[(uint16_t)(0x665a+jx_word(c,0x9334)*30u+npc)]=0;
    (void)JX_CALL(c,0x2f28,0x7978,npc);
    JxFarPointer after=erase_work(c,0xbb80);
    (void)JX_CALL(c,0x1832,0x1e88,x,y,pixels_w,pixels_h,after.offset,after.segment);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    before=erase_work(c,0x7d00);after=erase_work(c,0xbb80);
    (void)JX_CALL(c,0x17d2,0x01d4,x,y,pixels_w,pixels_h,before.offset,before.segment,
        after.offset,after.segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2),0x61de,c->data_segment);
}
