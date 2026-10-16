#include "event_engine.h"
/* 2F28:2CE4. Board challenge NPC state is signed, like the original CBW.
 * Read party slots after each level-up callback: those calls can change them. */
void jx_board_quest(JxEventContext *c,uint16_t npc){
    uint16_t state=(uint16_t)(0xcd42+npc);
    if((int8_t)jx_ds(c)[state]>4)return;
    if((int8_t)jx_ds(c)[state]==4){
        for(unsigned i=0;i<3;++i)(void)JX_CALL(c,0x1f2a,0x43b2,jx_word(c,(uint16_t)(0x875a+2*i)),100);
        jx_ds(c)[state]=6;
    }else if((uint16_t)JX_CALL(c,0x17d2,0x01da,(uint16_t)(int16_t)(int8_t)jx_ds(c)[state])){
        jx_ds(c)[state]=5;
    }else (void)JX_CALL(c,0x1f2a,0x8cea,45,1);
}
