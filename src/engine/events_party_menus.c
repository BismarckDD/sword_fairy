#include "event_engine.h"

/* 1F2A:AE12/A892. Each party slot is read immediately before its menu;
 * a menu callback may change the remaining party slots. */
void jx_party_actor_menus(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x864a,jx_word(c,0x875a));
    (void)JX_CALL(c,0x1f2a,0x864a,jx_word(c,0x875c));
    (void)JX_CALL(c,0x1f2a,0x864a,jx_word(c,0x875e));
}
void jx_party_skill_menus(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x9952,jx_word(c,0x875a));
    (void)JX_CALL(c,0x1f2a,0x9952,jx_word(c,0x875c));
    (void)JX_CALL(c,0x1f2a,0x9952,jx_word(c,0x875e));
}
static uint16_t actor_field(JxEventContext *c,uint16_t slot,uint16_t field) {
    return (uint16_t)(0xd04e + 68*jx_word(c,slot)+field);
}
static uint16_t choose_recipient(JxEventContext *c) {
    int second=jx_signed_word(jx_word(c,0x875c));
    if(second<0 || second>=5)return 0x875a;
    return (uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,
        actor_field(c,0x875a,0),c->data_segment,
        actor_field(c,0x875c,0),c->data_segment)!=0?0x875c:0x875a;
}
/* 1F2A:A8C0. Equipment is returned to the inventory before replacement.
 * Consumables update six fields in order, then clamp unsigned HP/MP. */
void jx_party_use_item(JxEventContext *c) {
    uint16_t item=(uint16_t)JX_CALL(c,0x1f2a,0x9182);
    int selected=jx_signed_word(item);
    if(selected>=12 && selected<38){(void)JX_CALL(c,0x1f2a,0x799a,item);return;}
    if(selected>0 && selected<12){
        uint16_t slot=choose_recipient(c);
        jx_set_word(c,0x8886,jx_word(c,actor_field(c,slot,0x26)));
        jx_set_word(c,0x8888,1);(void)JX_CALL(c,0x1f2a,0x8bd6);
        jx_set_word(c,actor_field(c,slot,0x26),item);
        (void)JX_CALL(c,0x1f2a,0x8f28,item,1);
    }
    if(selected<38 || selected>=59)return;
    uint16_t slot=choose_recipient(c);
    static const uint16_t source[]={0x10,0x14,0x16,0x18,0x0e,0x12};
    static const uint16_t target[]={0x14,0x16,0x18,0x1a,0x10,0x12};
    for(unsigned i=0;i<6;i++){
        uint16_t amount=jx_word(c,(uint16_t)(0x8022+28*item+source[i]));
        uint16_t at=actor_field(c,slot,target[i]);jx_set_word(c,at,(uint16_t)(jx_word(c,at)+amount));
    }
    if(jx_word(c,actor_field(c,slot,0x10))>jx_word(c,actor_field(c,slot,0x14)))
        jx_set_word(c,actor_field(c,slot,0x10),jx_word(c,actor_field(c,slot,0x14)));
    if(jx_word(c,actor_field(c,slot,0x12))>jx_word(c,actor_field(c,slot,0x16)))
        jx_set_word(c,actor_field(c,slot,0x12),jx_word(c,actor_field(c,slot,0x16)));
    (void)JX_CALL(c,0x1f2a,0x8f28,item,1);
}
