#include "event_engine.h"

/* 1F2A:747C. Negative actor indices do nothing; the original has no
 * positive upper-bound check. Poison halves unsigned maximum HP/MP. */
void jx_actor_restore_current(JxEventContext *c,uint16_t actor) {
    if ((int16_t)actor<0) return;
    uint16_t base=(uint16_t)(0xd04e +68u*actor);
    uint16_t hp=jx_word(c,(uint16_t)(base+0x14));
    uint16_t mp=jx_word(c,(uint16_t)(base+0x16));
    if (jx_word(c,(uint16_t)(base+0x24))==2) {hp/=2;mp/=2;}
    jx_set_word(c,(uint16_t)(base+0x10),hp);
    jx_set_word(c,(uint16_t)(base+0x12),mp);
}

/* 1F2A:43B2. text_work owns the original 66-byte message workspace.
 * Growth uses signed OLD level /10+1. The notification gate is read after
 * both string services, whose callbacks may change live game state. */
void jx_actor_level_up(JxEventContext *c,uint16_t actor,uint16_t notification,JxFarPointer text_work) {
    if ((int16_t)actor<0 || actor>4) return;
    uint16_t base=(uint16_t)(0xd04e +68u*actor);
    int tier=(int16_t)jx_word(c,(uint16_t)(base+0x22))/10+1;
    jx_set_word(c,(uint16_t)(base+0x22),(uint16_t)(jx_word(c,(uint16_t)(base+0x22))+1));
    const uint16_t fields[4]={0x14,0x16,0x18,0x1a};
    const int growth[4]={15,10,6,6};
    for (unsigned i=0;i<4;i++) {
        uint16_t field=(uint16_t)(base+fields[i]);
        jx_set_word(c,field,(uint16_t)(jx_word(c,field)+tier*growth[i]));
    }
    (void)JX_CALL(c,0x1f2a,0x747c,actor);
    (void)JX_CALL(c,0x1000,0x2448,text_work.offset,text_work.segment,base,c->data_segment);
    (void)JX_CALL(c,0x1000,0x23f4,text_work.offset,text_work.segment,0x129,c->data_segment);
    if (jx_word(c,0xacea)!=0)
        (void)JX_CALL(c,0x1f2a,0x728,100,notification,text_work.offset,text_work.segment);
}
