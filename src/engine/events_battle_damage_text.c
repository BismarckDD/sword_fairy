#include "event_engine.h"
static uint16_t damage_page(uint16_t counter,uint16_t advance) {
    return (uint16_t)(jx_signed_word((uint16_t)(counter+advance))%2);
}
static JxFarPointer damage_backup(JxEventContext *c) {
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+0x7ff4;
    JxFarPointer result={(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+((sum>>16)<<12))};return result;
}
static void damage_restore(JxEventContext *c,uint16_t x,uint16_t y) {
    JxFarPointer backup=damage_backup(c);
    (void)JX_CALL(c,0x1832,0x1c6c,x,y,40,17,backup.offset,backup.segment);
}
/* 1F2A:2E10. work preserves the20-byte original frame. Invalid sides
 * retain its incoming coordinates at14/16, matching uninitialized BP
 * locals rather than inventing a default position. Reserve it disjointly. */
void jx_battle_show_damage(JxEventContext *c,uint16_t side,uint16_t amount,JxFarPointer work) {
    (void)JX_CALL(c,0x1f2a,0x06f2,47,0,77);
    uint16_t x=jx_far_word(c,work.segment,(uint16_t)(work.offset+14));
    uint16_t y=jx_far_word(c,work.segment,(uint16_t)(work.offset+16));
    if(side==0){x=55;y=45;}
    else if(side==1){x=(uint16_t)(jx_word(c,0x292c)+140);y=(uint16_t)(jx_word(c,0x292e)+70);}
    else if(side==2){x=240;y=110;}
    (void)JX_CALL(c,0x1000,0x2a8c,work.offset,work.segment,0x00bc,c->data_segment,amount);
    for(unsigned frame=0;frame<16;frame++) {
        uint16_t counter=jx_word(c,0x3e54);
        (void)JX_CALL(c,0x1832,0x135e,damage_page(counter,1),damage_page(counter,0));
        (void)JX_CALL(c,0x1832,0x1810,damage_page(jx_word(c,0x3e54),1));
        if(frame>0)damage_restore(c,x,(uint16_t)(y+2));
        JxFarPointer backup=damage_backup(c);
        (void)JX_CALL(c,0x1832,0x1e88,x,y,40,17,backup.offset,backup.segment);
        (void)JX_CALL(c,0x1f2a,0x0728,x,y,work.offset,work.segment);
        (void)JX_CALL(c,0x1832,0x1836,damage_page(jx_word(c,0x3e54),1));
        (void)JX_CALL(c,0x1f2a,0x2dbc,1);
        jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
        y=(uint16_t)(y-2);
    }
    damage_restore(c,x,(uint16_t)(y+2));
    (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
}
