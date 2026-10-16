/* Recovered high-level battle turn logic; see battle_turn_verification.json. */
#include "event_engine.h"
static uint16_t turn_actor_field(JxEventContext *c,uint16_t global,uint16_t field) {
    return jx_word(c,(uint16_t)(0xd04e + jx_word(c,global)*68+field));
}
static uint16_t turn_selected(JxEventContext *c,uint16_t global,uint16_t table,uint16_t field) {
    return jx_word(c,(uint16_t)(table+jx_word(c,global)*28+field));
}
static uint16_t turn_equipment(JxEventContext *c,uint16_t global,uint16_t field) {
    return jx_word(c,(uint16_t)(0x8022+turn_actor_field(c,global,0x26)*28+field));
}
static uint16_t turn_next_page(JxEventContext *c) {
    return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2);
}
static JxFarPointer turn_backup(JxEventContext *c) {
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+0x7ff4;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+((sum>>16)<<12))};
}
static void turn_message_copy(JxEventContext *c,JxFarPointer message,uint16_t source) {
    (void)JX_CALL(c,0x1000,0x2448,message.offset,message.segment,source,c->data_segment);
}
static uint32_t turn_prepare_actor(JxEventContext *c,uint16_t side,JxFarPointer message) {
    uint16_t actor=side==1?0x2936:0x2938,action=side==1?0x61d6:0x61da,selection=(uint16_t)(action+2);
    switch(jx_word(c,action)) {
    case 0: {
        int critical=(uint16_t)JX_CALL(c,0x1f2a,0x71de,5)!=0;
        turn_message_copy(c,message,side==1?(critical?0x00c0:0x00c9):(critical?0x00d7:0x00e0));
        uint16_t defense=critical?0:jx_word(c,0x6618);
        uint16_t attack=turn_actor_field(c,actor,0x18);
        attack=(uint16_t)(attack+turn_equipment(c,actor,0x16));
        uint32_t damage=(uint32_t)attack-defense;
        (void)JX_CALL(c,0x1f2a,0x3d1c,side,turn_actor_field(c,actor,0x28));
        return damage==0 || (damage&0x80000000u)?1:damage;
    }
    case 1:
        turn_message_copy(c,message,(uint16_t)(0x8022+jx_word(c,selection)*28));
        (void)JX_CALL(c,0x1f2a,0x3d1c,side,turn_actor_field(c,actor,0x20));
        return turn_selected(c,selection,0x8022,0x0e);
    case 3: {
        turn_message_copy(c,message,(uint16_t)(0x5154+jx_word(c,selection)*28));
        (void)JX_CALL(c,0x1f2a,0x3d1c,side,turn_selected(c,selection,0x5154,0x18));
        uint32_t damage=(uint32_t)(int32_t)jx_signed_word(turn_selected(c,selection,0x5154,0x10));
        if(side==1)(void)JX_CALL(c,0x1f2a,0x5fd8,turn_selected(c,selection,0x5154,0x1a),10);
        return damage;
    }
    default:
        if(side==1)turn_message_copy(c,message,0x00ce);
        return 0;
    }
}
static uint16_t turn_panel_x(JxEventContext *c,uint16_t side) {
    return side==0?80:(side==1?(uint16_t)(jx_word(c,0x292c)+140):240);
}
static uint16_t turn_panel_y(JxEventContext *c,uint16_t side) {
    return side==0?30:(side==1?(uint16_t)(jx_word(c,0x292e)+70):110);
}
static void turn_show_message(JxEventContext *c,uint16_t side,JxFarPointer message) {
    JxFarPointer backup=turn_backup(c);
    (void)JX_CALL(c,0x1832,0x1e88,turn_panel_x(c,side),turn_panel_y(c,side),64,17,backup.offset,backup.segment);
    (void)JX_CALL(c,0x1f2a,0x0728,turn_panel_x(c,side),turn_panel_y(c,side),message.offset,message.segment);
    (void)JX_CALL(c,0x1f2a,0x2dbc,16);
    backup=turn_backup(c);
    (void)JX_CALL(c,0x1832,0x1c6c,turn_panel_x(c,side),turn_panel_y(c,side),64,17,backup.offset,backup.segment);
}
static uint16_t turn_random(JxEventContext *c) {return (uint16_t)JX_CALL0(c,0x1000,0x32b2);}
static uint32_t turn_scale(uint32_t damage,uint16_t random) {
    uint32_t product=jx_multiply_u32(damage,(uint32_t)(jx_signed_word(random)%20+90)),result=0;
    (void)jx_divide_signed32(product,100,&result);return result;
}
static void turn_map(JxEventContext *c,uint16_t first,uint16_t count) {
    for(uint16_t slot=0;slot<count;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
static void turn_background(JxEventContext *c) {
    (void)JX_CALL(c,0x1832,0x1810,turn_next_page(c));turn_map(c,135,4);
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
}
static void turn_show_page(JxEventContext *c,uint16_t ticks) {
    (void)JX_CALL(c,0x1832,0x1836,turn_next_page(c));
    (void)JX_CALL(c,0x1f2a,0x2dbc,ticks);
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
}
static void turn_standing(JxEventContext *c) {
    turn_background(c);(void)JX_CALL(c,0x1f2a,0x2a9e,0);
    if(jx_word(c,0x2936)!=jx_word(c,0x2938))(void)JX_CALL(c,0x1f2a,0x2a9e,2);
    (void)JX_CALL(c,0x1f2a,0x2a9e,1);turn_show_page(c,3);
}
static uint16_t turn_sprite_x(JxEventContext *c,uint16_t side) {
    return side==0?40:(side==1?jx_word(c,0x292c):100);
}
static uint16_t turn_sprite_y(JxEventContext *c,uint16_t side) {
    return side==0?10:(side==1?jx_word(c,0x292e):40);
}
static void turn_blink(JxEventContext *c,uint16_t side) {
    for(unsigned flash=0;flash<4;flash++) {
        (void)JX_CALL(c,0x1832,0x1576,turn_sprite_x(c,side),turn_sprite_y(c,side),220,148,
                      jx_word(c,0x5858),jx_word(c,0x585a),255,101);
        (void)JX_CALL(c,0x1f2a,0x2dbc,2);
        (void)JX_CALL(c,0x1832,0x1a28,turn_sprite_x(c,side),turn_sprite_y(c,side),220,148,
                      jx_word(c,0x5858),jx_word(c,0x585a),255);
        (void)JX_CALL(c,0x1f2a,0x2dbc,2);
    }
}
/* 1F2A:71DE. Random is interpreted as signed16 even though the normal
 * runtime generator produces a nonnegative value. */
uint16_t jx_chance_percent(JxEventContext *c,uint16_t percent) {
    return jx_signed_word(turn_random(c))%100<jx_signed_word(percent)?1:0;
}
/* 1F2A:2FC6, complete turn controller. The52-byte work retains the far
 * message ABI at+10; arithmetic state is represented by named C locals.
 * Input names/messages must fit the original short stack string buffer. */
void jx_battle_run_turn(JxEventContext *c,JxFarPointer work) {
    JxFarPointer message={(uint16_t)(work.offset+10),work.segment};
    uint32_t first_damage=turn_prepare_actor(c,1,message),second_damage=0;
    turn_show_message(c,1,message);
    if(jx_word(c,0x2936)!=jx_word(c,0x2938)) {
        second_damage=turn_prepare_actor(c,2,message);turn_show_message(c,2,message);
    }
    int target=0;
    if(jx_word(c,0x2936)!=jx_word(c,0x2938))target=jx_signed_word(turn_random(c))%2;
    uint32_t enemy_damage;
    if(jx_signed_word(turn_random(c))%2!=0) {
        turn_message_copy(c,message,0x00e5);
        (void)JX_CALL(c,0x1f2a,0x3d1c,0,jx_word(c,0x661a));
        uint16_t random=turn_random(c),actor=target==0?0x2936:0x2938;
        uint16_t defense=turn_actor_field(c,actor,0x1a);
        defense=(uint16_t)(defense+turn_equipment(c,actor,0x18));
        enemy_damage=turn_scale((uint32_t)jx_word(c,0x6616)-defense,random);
        if(enemy_damage==0 || (enemy_damage&0x80000000u))enemy_damage=1;
    } else {
        turn_message_copy(c,message,(uint16_t)(0x5154+jx_word(c,0x661e)*28));
        (void)JX_CALL(c,0x1f2a,0x3d1c,0,turn_selected(c,0x661e,0x5154,0x18));
        uint16_t random=turn_random(c);
        enemy_damage=turn_scale((uint32_t)(int32_t)jx_signed_word(turn_selected(c,0x661e,0x5154,0x10)),random);
    }
    turn_show_message(c,0,message);
    (void)JX_CALL(c,0x1f2a,0x155c,jx_word(c,0x61d6)==3?10:6);
    for(uint16_t frame=0;frame<8;frame++) {
        turn_background(c);(void)JX_CALL(c,0x1f2a,0x2c9c,0,frame);
        if(jx_word(c,0x2936)!=jx_word(c,0x2938)) {
            if(jx_word(c,0x61da)==4 || frame==0)(void)JX_CALL(c,0x1f2a,0x2a9e,2);
            else (void)JX_CALL(c,0x1f2a,0x2c9c,2,frame);
        }
        if(jx_word(c,0x61d6)==4 || frame==0)(void)JX_CALL(c,0x1f2a,0x2a9e,1);
        else (void)JX_CALL(c,0x1f2a,0x2c9c,1,frame);
        turn_show_page(c,2);
    }
    turn_standing(c);
    if(jx_word(c,0x2936)==0 && jx_word(c,0x61d8)==30 && jx_word(c,0x2934)==15) {
        jx_set_word(c,0xacea,0);return;
    }
    turn_map(c,87,2);
    uint32_t total_damage=turn_scale(first_damage+second_damage,turn_random(c));
    jx_set_word(c,0x6614,(uint16_t)(jx_word(c,0x6614)-(uint16_t)total_damage));
    if(total_damage!=0 && !(total_damage&0x80000000u)) {
        turn_blink(c,0);(void)JX_CALL(c,0x1f2a,0x2e10,0,(uint16_t)total_damage);
        if(jx_signed_word(jx_word(c,0x6614))<=0)return;
    }
    uint16_t actor=target==0?0x2936:0x2938,side=target==0?1:2;
    uint16_t hp=turn_actor_field(c,actor,0x10),maximum=turn_actor_field(c,actor,0x14);
    turn_map(c,hp>=maximum/4?(side==1?103:119):(side==1?143:147),2);
    turn_blink(c,side);
    uint16_t field=(uint16_t)(0xd04e + jx_word(c,actor)*68+0x10);
    jx_set_word(c,field,(uint16_t)(jx_word(c,field)-(uint16_t)enemy_damage));
    (void)JX_CALL(c,0x1f2a,0x2e10,side,(uint16_t)enemy_damage);
    if(jx_signed_word(turn_actor_field(c,actor,0x10))<=0)return;
    turn_standing(c);
}
