#include "event_engine.h"

static JxFarPointer choice_buffer(JxEventContext *c,uint16_t displacement) {
    uint32_t offset=(uint32_t)jx_word(c,0x2eb0)+displacement;
    return (JxFarPointer){(uint16_t)offset,(uint16_t)(jx_word(c,0x2eb2)+((offset>>16)<<12))};
}
static void choice_background(JxEventContext *c,uint16_t entry,uint16_t x,uint16_t y) {
    JxFarPointer p=choice_buffer(c,32000);
    (void)JX_CALL(c,0x1832,entry,x,y,43,20,p.offset,p.segment);
}
static void choice_marker(JxEventContext *c,uint16_t x,uint16_t y) {
    (void)JX_CALL(c,0x1832,0x1a28,x,y,43,20,jx_word(c,0x888e),jx_word(c,0x8890),255);
}
/* 1F2A:7BAA. Two text choices, keyboard up/down and Enter/Space.
 * The original ignores Escape and other events, and has no timeout.
 * event_work is the original eight-byte input record, separate from selection. */
uint16_t jx_two_choice_menu(JxEventContext *c,uint16_t x,uint16_t y,
    JxFarPointer first,JxFarPointer second,JxFarPointer event_work) {
    uint16_t selection=0;
    (void)JX_CALL(c,0x1832,0x1e88,x,y,72,40,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    JxFarPointer mapped=choice_buffer(c,2880);
    (void)JX_CALL(c,0x2f28,0x6c7e,jx_word(c,0x2eb0),jx_word(c,0x2eb2),mapped.offset,mapped.segment,2880,0xb584,c->data_segment);
    mapped=choice_buffer(c,2880);
    (void)JX_CALL(c,0x1832,0x1c6c,x,y,72,40,mapped.offset,mapped.segment);
    (void)JX_CALL(c,0x1f2a,0x0728,x,y,first.offset,first.segment);
    (void)JX_CALL(c,0x1f2a,0x0728,x,(uint16_t)(y+20),second.offset,second.segment);
    choice_background(c,0x1e88,(uint16_t)(x+40),y);
    choice_marker(c,(uint16_t)(x+40),y);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event_work.offset,event_work.segment)!=0) {}
        uint16_t type=jx_far_word(c,event_work.segment,event_work.offset);
        uint16_t key=jx_far_word(c,event_work.segment,(uint16_t)(event_work.offset+2));
        if(type==1 && (key==0x50 || key==0x48)) {
            choice_background(c,0x1c6c,(uint16_t)(x+40),(uint16_t)(y+20*selection));
            selection=(uint16_t)((selection+1)%2);
            choice_background(c,0x1e88,(uint16_t)(x+40),(uint16_t)(y+20*selection));
            choice_marker(c,(uint16_t)(x+40),(uint16_t)(y+20*selection));
        }
        /* Read again after callbacks, as the original reloads its record. */
        type=jx_far_word(c,event_work.segment,event_work.offset);
        key=jx_far_word(c,event_work.segment,(uint16_t)(event_work.offset+2));
        if(type==0 && (key==0x1c || key==0x39)) {
            choice_background(c,0x1c6c,(uint16_t)(x+40),(uint16_t)(y+20*selection));
            (void)JX_CALL(c,0x1832,0x1c6c,x,y,72,40,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
            return selection;
        }
    }
}
