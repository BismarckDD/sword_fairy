#include "event_engine.h"
static JxFarPointer choice_buffer(JxEventContext *c,uint16_t advance) {
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+advance;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+((sum>>16)<<12))};
}
static void choice_image(JxEventContext *c,uint16_t entry,uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t advance) {
    JxFarPointer p=choice_buffer(c,advance);
    (void)JX_CALL(c,0x1832,entry,x,y,w,h,p.offset,p.segment);
}
static void choice_cursor(JxEventContext *c,uint16_t x,uint16_t y) {
    (void)JX_CALL(c,0x1832,0x1a28,x,y,43,20,jx_word(c,0x888e),jx_word(c,0x8890),255);
}
/* 1F2A:7E0E. Three text choices, wrapping up/down selection.
 * Eight private event bytes supplied by the caller replace compiler stack. */
uint16_t jx_choice_menu(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer first,JxFarPointer second,JxFarPointer third,JxFarPointer event) {
    uint16_t selected=0;
    choice_image(c,0x1e88,x,y,100,60,0);
    JxFarPointer source=choice_buffer(c,0),dest=choice_buffer(c,6000);
    (void)JX_CALL(c,0x2f28,0x6c7e,source.offset,source.segment,dest.offset,dest.segment,6000,0xb584,c->data_segment);
    choice_image(c,0x1c6c,x,y,100,60,6000);
    (void)JX_CALL(c,0x1f2a,0x0728,x,y,first.offset,first.segment);
    (void)JX_CALL(c,0x1f2a,0x0728,x,(uint16_t)(y+20),second.offset,second.segment);
    (void)JX_CALL(c,0x1f2a,0x0728,x,(uint16_t)(y+40),third.offset,third.segment);
    choice_image(c,0x1e88,(uint16_t)(x+49),y,43,20,32000);
    choice_cursor(c,(uint16_t)(x+49),y);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event.offset,event.segment)!=0) {}
        uint16_t type=jx_far_word(c,event.segment,event.offset);
        uint16_t key=jx_far_word(c,event.segment,(uint16_t)(event.offset+2));
        if(type==1 && (key==0x50 || key==0x48)) {
            choice_image(c,0x1c6c,(uint16_t)(x+49),(uint16_t)(y+selected*20),43,20,32000);
            selected=(uint16_t)((selected+(key==0x50?1:2))%3);
            choice_image(c,0x1e88,(uint16_t)(x+49),(uint16_t)(y+selected*20),43,20,32000);
            choice_cursor(c,(uint16_t)(x+49),(uint16_t)(y+selected*20));
        }
        /* Re-read the private event after drawing callbacks, as original. */
        type=jx_far_word(c,event.segment,event.offset);
        key=jx_far_word(c,event.segment,(uint16_t)(event.offset+2));
        if(type==0 && (key==0x1c || key==0x39)) {
            choice_image(c,0x1c6c,x,y,100,60,0);return selected;
        }
    }
}
