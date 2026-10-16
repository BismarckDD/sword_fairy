#include "event_engine.h"
static uint16_t action_slot(JxEventContext *c) {return jx_word(c,0x2930)==0?0x61d6:0x61da;}
static uint16_t actor_slot(uint16_t action) {return action==0x61d6?0x2936:0x2938;}
static uint16_t action_actor_field(JxEventContext *c,uint16_t global,uint16_t field) {
    return (uint16_t)(0xd04e + jx_word(c,global)*0x44+field);
}
static uint16_t item_effect(JxEventContext *c,uint16_t global,uint16_t field) {
    return jx_word(c,(uint16_t)(0x8022+jx_word(c,global)*0x1c+field));
}
static uint16_t skill_cost(JxEventContext *c,uint16_t global) {
    return jx_word(c,(uint16_t)(0x5154+jx_word(c,global)*0x1c+0x14));
}
/* 1F2A:3E88 ignores all four original stack arguments. Turn computation
 * uses globals established by the menus and is skipped when ACEA iszero. */
void jx_battle_turn_dispatch(JxEventContext *c) {
    if(jx_word(c,0xacea)!=0)(void)JX_CALL0(c,0x1f2a,0x2fc6);
}
/* 1F2A:3EEE. Any nonzero2930 selects the second actor's command. */
void jx_battle_choose_attack(JxEventContext *c) {jx_set_word(c,action_slot(c),0);}
/* 1F2A:3EB2. Entry side is retained even if the actor menu changes2930. */
void jx_battle_open_actor_menu(JxEventContext *c) {
    uint16_t action=action_slot(c);jx_set_word(c,action,2);
    (void)JX_CALL(c,0x1f2a,0x864a,jx_word(c,actor_slot(action)));
}
/* 1F2A:3F12. Immediate item effects and unsigned resource ceilings.
 * Re-read item/actor indices at each original field update: data aliasing
 * can alter them between writes even without another engine callback. */
void jx_battle_choose_item(JxEventContext *c) {
    uint16_t action=action_slot(c),selection=(uint16_t)(action+2),actor=actor_slot(action);
    jx_set_word(c,action,1);jx_set_word(c,selection,(uint16_t)JX_CALL0(c,0x1f2a,0x9182));
    int item=jx_signed_word(jx_word(c,selection));
    if(item>58 || (item>0 && item<12)) {
        (void)JX_CALL(c,0x1f2a,0x8f28,jx_word(c,selection),1);return;
    }
    jx_set_word(c,action,2);item=jx_signed_word(jx_word(c,selection));
    if(item<38 || item>=59)return;
    static const uint16_t source_field[]={0x10,0x14,0x16,0x18,0x0e,0x12};
    static const uint16_t target_field[]={0x14,0x16,0x18,0x1a,0x10,0x12};
    for(unsigned k=0;k<6;k++) {
        uint16_t amount=item_effect(c,selection,source_field[k]);
        uint16_t at=action_actor_field(c,actor,target_field[k]);
        jx_set_word(c,at,(uint16_t)(jx_word(c,at)+amount));
    }
    uint16_t hp=action_actor_field(c,actor,0x10),maximum=action_actor_field(c,actor,0x14);
    if(jx_word(c,hp)>jx_word(c,maximum))jx_set_word(c,action_actor_field(c,actor,0x10),jx_word(c,action_actor_field(c,actor,0x14)));
    uint16_t mp=action_actor_field(c,actor,0x12);maximum=action_actor_field(c,actor,0x16);
    if(jx_word(c,mp)>jx_word(c,maximum))jx_set_word(c,action_actor_field(c,actor,0x12),jx_word(c,action_actor_field(c,actor,0x16)));
    (void)JX_CALL(c,0x1f2a,0x8f28,jx_word(c,selection),1);
}
/* 1F2A:4288. Negative skill cancels; MP/cost comparison is signed16.
 * Cost is re-read for subtraction, and insufficient MP selects command2. */
void jx_battle_choose_skill(JxEventContext *c) {
    uint16_t action=action_slot(c),selection=(uint16_t)(action+2),actor=actor_slot(action);
    jx_set_word(c,action,3);jx_set_word(c,selection,(uint16_t)JX_CALL(c,0x1f2a,0x9952,jx_word(c,actor)));
    if(jx_signed_word(jx_word(c,selection))<0) {jx_set_word(c,action,2);return;}
    uint16_t cost=skill_cost(c,selection),mp=action_actor_field(c,actor,0x12);
    if(jx_signed_word(jx_word(c,mp))<jx_signed_word(cost)) {
        jx_set_word(c,action,2);
        (void)JX_CALL(c,0x2f28,0x648e,0xffff,action==0x61d6?0x011c:0x010f,c->data_segment,0);return;
    }
    cost=skill_cost(c,selection);mp=action_actor_field(c,actor,0x12);
    jx_set_word(c,mp,(uint16_t)(jx_word(c,mp)-cost));
}
