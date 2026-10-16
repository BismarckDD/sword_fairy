#include "event_engine.h"
static void wager_draw(JxEventContext *c,uint16_t unit,uint16_t multiplier,uint16_t format,JxFarPointer work) {
    uint16_t amount=(uint16_t)((uint32_t)unit*multiplier);
    (void)JX_CALL(c,0x1000,0x2a8c,work.offset,work.segment,format,c->data_segment,amount);
    (void)JX_CALL(c,0x1832,0x1884,20,160,32,20,255);
    (void)JX_CALL(c,0x1f2a,0x0728,20,160,work.offset,work.segment);
    uint16_t source=(uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x2952)+1))%2+2);
    uint16_t dest=(uint16_t)(jx_signed_word(jx_word(c,0x2952))%2+2);
    (void)JX_CALL(c,0x1832,0x135e,source,dest);
}
/* 1F2A:B522.20 private bytes: text at0 (8 bytes), event at10 (8).
 * Comparisons use signed, truncated 16-bit products exactly as original. */
uint16_t jx_dice_choose_wager(JxEventContext *c,uint16_t unit,uint16_t player,uint16_t opponent,JxFarPointer work) {
    uint16_t multiplier=1;
    JxFarPointer event={(uint16_t)(work.offset+10),work.segment};
    wager_draw(c,unit,multiplier,0x0751,work);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event.offset,event.segment)!=0) {}
        uint16_t type=jx_far_word(c,event.segment,event.offset),key=jx_far_word(c,event.segment,(uint16_t)(event.offset+2));
        if(type==1) {
            if(key==0x50) {if(jx_signed_word(multiplier)>1)multiplier--;}
            else if(key==0x48) {
                uint16_t amount=(uint16_t)((uint32_t)(uint16_t)(multiplier+1)*unit);
                if(jx_signed_word(amount)<=jx_signed_word(player) && jx_signed_word(amount)<=jx_signed_word(opponent))multiplier++;
            }
            wager_draw(c,unit,multiplier,0x0755,work);
        }
        type=jx_far_word(c,event.segment,event.offset);key=jx_far_word(c,event.segment,(uint16_t)(event.offset+2));
        if(type==0 && (key==0x1c || key==0x39))return (uint16_t)((uint32_t)unit*multiplier);
    }
}
