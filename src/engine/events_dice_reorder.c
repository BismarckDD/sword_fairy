#include "event_engine.h"
static void dice_store(JxEventContext *c,JxFarPointer p,uint16_t value) {
    uint32_t at=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 1F2A:B4BA. Put a matching pair first. This is not a value sort.
 * Re-read after each swap to preserve overlapping-pointer semantics. */
void jx_dice_reorder(JxEventContext *c,JxFarPointer first,JxFarPointer second,JxFarPointer third) {
    if(jx_far_word(c,second.segment,second.offset)==jx_far_word(c,third.segment,third.offset)) {
        uint16_t saved=jx_far_word(c,first.segment,first.offset);
        dice_store(c,first,jx_far_word(c,third.segment,third.offset));dice_store(c,third,saved);
    }
    if(jx_far_word(c,first.segment,first.offset)==jx_far_word(c,third.segment,third.offset)) {
        uint16_t saved=jx_far_word(c,second.segment,second.offset);
        dice_store(c,second,jx_far_word(c,third.segment,third.offset));dice_store(c,third,saved);
    }
}
