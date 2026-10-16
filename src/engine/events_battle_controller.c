#include "event_engine.h"
#include <string.h>
static uint16_t battle_page(JxEventContext *c,unsigned next) {
    return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+next))%2);
}
static uint16_t battle_actor_field(JxEventContext *c,uint16_t actor_global,uint16_t field) {
    return jx_word(c,(uint16_t)(0xd04e + jx_word(c,actor_global)*0x44 + field));
}
static void battle_maps(JxEventContext *c,uint16_t first,unsigned count) {
    for(unsigned slot=0;slot<count;slot++)(void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
static void battle_rect(JxEventContext *c,uint16_t function,uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t global,uint16_t shift) {
    (void)JX_CALL(c,0x1832,function,x,y,w,h,(uint16_t)(jx_word(c,global)+shift),jx_word(c,(uint16_t)(global+2)));
}
static void battle_palette(JxEventContext *c,uint16_t destination) {
    double gain;float weight;uint32_t bits;
    memcpy(&gain,c->memory+((uint32_t)c->data_segment*16)+0x2384,sizeof(gain));
    weight=(float)gain;memcpy(&bits,&weight,sizeof(bits));
    (void)JX_CALL(c,0x2f28,0xa8d0,destination,c->data_segment,jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,(uint16_t)bits,(uint16_t)(bits>>16));
}
static void battle_font(JxEventContext *c,uint16_t color) {(void)JX_CALL(c,0x1f2a,0x06f2,color,0,0x4d);}
static void battle_callbacks(JxEventContext *c) {
    static const uint16_t entry[]={0x3eee,0x3eb2,0x3f12,0x4288};
    for(unsigned k=0;k<4;k++) {jx_set_word(c,(uint16_t)(0x8722 + 4*k),entry[k]);jx_set_word(c,(uint16_t)(0x8724 + 4*k),0x1f2a);}
}
static int battle_cd_enabled(JxEventContext *c) {return jx_signed_word(jx_word(c,0x31b4))>0 && jx_signed_word(jx_word(c,0x050c))>0;}
static void battle_cd_cue(JxEventContext *c,uint16_t track_delta,uint16_t length_offset) {
    uint16_t length=jx_far_word(c,jx_word(c,0x2382),length_offset);
    (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track_delta)&255),2,length);
}
static void battle_private_word(JxEventContext *c,JxFarPointer p,uint16_t field,uint16_t value) {
    uint32_t at=(uint32_t)p.segment*16+(uint16_t)(p.offset+field);
    c->memory[at&0xfffff]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 1F2A:4934. Encounter record, first actor, signed optional second actor.
 * menu_work owns20 disjoint private bytes: four Far menu strings and NULL.
 * Returned word:0 enemy HP <=0,FFFF escape,1 allied actor HP <=0.
 * Engine operations use their
 * original address services; this controller contains no instruction VM. */
uint16_t jx_battle_run(JxEventContext *c,uint16_t encounter,uint16_t first_actor,uint16_t second_actor,JxFarPointer menu_work) {
    static const uint16_t labels[]={0x017d,0x018b,0x0199,0x01a7};
    for(unsigned k=0;k<4;k++) {battle_private_word(c,menu_work,(uint16_t)(4*k),labels[k]);battle_private_word(c,menu_work,(uint16_t)(4*k+2),c->data_segment);}
    (void)JX_CALL(c,0x1000,0x3572,(uint16_t)(menu_work.offset+16),menu_work.segment,0,4);
    if(battle_cd_enabled(c)) {(void)JX_CALL0(c,0x1832,0x22ce);battle_cd_cue(c,14,0x052c);}
    jx_set_word(c,0x2932,0);
    uint16_t source=(uint16_t)(0x499e + encounter*30);
    for(unsigned k=0;k<15;k++)jx_set_word(c,(uint16_t)(0x6606 + 2*k),jx_word(c,(uint16_t)(source+2*k)));
    jx_set_word(c,0x2934,encounter);jx_set_word(c,0x2936,first_actor);
    if(jx_signed_word(second_actor)>=0) {jx_set_word(c,0x292c,40);jx_set_word(c,0x292e,50);jx_set_word(c,0x2938,second_actor);}
    else {jx_set_word(c,0x292c,70);jx_set_word(c,0x292e,45);jx_set_word(c,0x2938,first_actor);}
    (void)JX_CALL(c,0x1f2a,0x5fd8,battle_actor_field(c,0x2936,0x2a),6);
    (void)JX_CALL(c,0x1f2a,0x5fd8,battle_actor_field(c,0x2936,0x30),7);
    (void)JX_CALL(c,0x1f2a,0x5fd8,battle_actor_field(c,0x2938,0x30),8);
    (void)JX_CALL(c,0x1f2a,0x5fd8,10,9);
    jx_set_word(c,0xacea,1);battle_font(c,80);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1810,battle_page(c,1));
    (void)JX_CALL(c,0x1f2a,0x4830,143,battle_actor_field(c,0x2936,0x32));
    (void)JX_CALL(c,0x1f2a,0x4830,145,battle_actor_field(c,0x2936,0x2e));
    (void)JX_CALL(c,0x1f2a,0x4830,147,battle_actor_field(c,0x2938,0x32));
    (void)JX_CALL(c,0x1f2a,0x4830,149,battle_actor_field(c,0x2938,0x2e));
    (void)JX_CALL(c,0x1f2a,0x46e6,jx_word(c,0x9366));battle_palette(c,0x2eb4);
    battle_rect(c,0x1c6c,0,0,320,200,0x5858,0);
    (void)JX_CALL(c,0x1f2a,0x3d1c,0,jx_word(c,0x661a));
    (void)JX_CALL(c,0x1f2a,0x3d1c,1,jx_word(c,(uint16_t)(0xd04e + first_actor*0x44 + 0x28)));
    if(jx_signed_word(second_actor)>=0)(void)JX_CALL(c,0x1f2a,0x3d1c,2,jx_word(c,(uint16_t)(0xd04e + second_actor*0x44 + 0x28)));
    battle_maps(c,69,2);battle_rect(c,0x1e88,40,10,220,148,0x5858,0);
    (void)JX_CALL(c,0x1f2a,0x2c9c,0,0);
    battle_maps(c,139,4);battle_rect(c,0x1e88,100,40,220,148,0x5858,0x7f30);
    if(jx_signed_word(second_actor)>=0)(void)JX_CALL(c,0x1f2a,0x2c9c,2,0);
    battle_maps(c,139,4);battle_rect(c,0x1e88,jx_word(c,0x292c),jx_word(c,0x292e),220,148,0x5858,0);
    (void)JX_CALL(c,0x1f2a,0x2c9c,1,0);
    (void)JX_CALL(c,0x1832,0x1836,battle_page(c,1));jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    (void)JX_CALL0(c,0x1f2a,0xc24e);battle_font(c,47);
    battle_rect(c,0x1e88,90,30,104,17,0x2eb0,0);
    (void)JX_CALL(c,0x1f2a,0x0728,90,30,(uint16_t)(0x499e + encounter*30),c->data_segment);
    (void)JX_CALL(c,0x1f2a,0x72b2,20);battle_rect(c,0x1c6c,90,30,104,17,0x2eb0,0);battle_font(c,80);
    uint16_t result;
    for(;;) {
        for(;;) {
            jx_set_word(c,0x2930,0);battle_callbacks(c);
            uint16_t accepted=(uint16_t)JX_CALL(c,0x1f2a,0x80fa,0,120,4,menu_work.offset,menu_work.segment);
            if(jx_signed_word(accepted)>=0 && jx_word(c,0x61d6)!=2 && jx_signed_word(jx_word(c,0x61d6))>=0)break;
        }
        if(jx_signed_word(second_actor)>=0) {
            for(;;) {
                jx_set_word(c,0x2930,1);battle_callbacks(c);
                uint16_t accepted=(uint16_t)JX_CALL(c,0x1f2a,0x80fa,248,120,4,menu_work.offset,menu_work.segment);
                if(jx_signed_word(accepted)>=0 && jx_word(c,0x61da)!=2 && jx_signed_word(jx_word(c,0x61da))>=0)break;
            }
        }
        (void)JX_CALL(c,0x1f2a,0x3e88,jx_word(c,0x61d6),jx_word(c,0x61d8),jx_word(c,0x61da),jx_word(c,0x61dc));
        jx_set_word(c,0x2932,(uint16_t)(jx_word(c,0x2932)+1));
        if(jx_word(c,0x2932)==10 && jx_word(c,0x2936)==3 && jx_word(c,0x2934)==28)jx_set_word(c,0xacea,0);
        if(jx_signed_word(jx_word(c,0x6614))<=0) {
            if(battle_cd_enabled(c))battle_cd_cue(c,19,0x0536);
            battle_maps(c,69,2);battle_rect(c,0x1c6c,40,10,220,108,0x5858,0);battle_font(c,47);
            (void)JX_CALL0(c,0x1f2a,0x44ac);result=0;(void)JX_CALL0(c,0x1f2a,0x7272);break;
        }
        if(jx_word(c,0xacea)==0) {
            battle_maps(c,139,4);battle_rect(c,0x1c6c,jx_word(c,0x292c),jx_word(c,0x292e),220,148,0x5858,0);
            battle_rect(c,0x1c6c,100,40,220,148,0x5858,0x7f30);
            (void)JX_CALL(c,0x1f2a,0x2c9c,0,0);result=0xffff;(void)JX_CALL(c,0x1f2a,0x72b2,18);break;
        }
        if(jx_signed_word(battle_actor_field(c,0x2936,0x10))>0 && jx_signed_word(battle_actor_field(c,0x2938,0x10))>0)continue;
        (void)JX_CALL(c,0x1f2a,0x155c,jx_signed_word(battle_actor_field(c,0x2936,0x10))<=0?7:8);
        (void)JX_CALL(c,0x1832,0x1810,battle_page(c,1));battle_maps(c,135,4);battle_rect(c,0x1c6c,0,0,320,200,0x5858,0);
        (void)JX_CALL(c,0x1f2a,0x2a9e,0);
        if(jx_word(c,0x2938)!=jx_word(c,0x2936))(void)JX_CALL(c,0x1f2a,0x2a9e,2);
        (void)JX_CALL(c,0x1f2a,0x2a9e,1);
        (void)JX_CALL(c,0x1832,0x1836,battle_page(c,1));jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));battle_font(c,47);
        (void)JX_CALL(c,0x1f2a,0x0728,100,96,0x01ae,c->data_segment);result=1;(void)JX_CALL0(c,0x1f2a,0x7272);break;
    }
    jx_set_word(c,0x8722,0x86ba);jx_set_word(c,0x8724,0x2f28);
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1810,battle_page(c,1));(void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
    (void)JX_CALL(c,0x1832,0x1810,battle_page(c,0));(void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
    (void)JX_CALL0(c,0x1f2a,0x5ae8);battle_font(c,80);(void)JX_CALL0(c,0x2f28,0x5580);
    if(battle_cd_enabled(c)) {
        (void)JX_CALL0(c,0x1832,0x22f2);
        uint16_t place=jx_word(c,0x889a);
        if(jx_signed_word(place)>=0)battle_cd_cue(c,place,(uint16_t)(0x0510 + place*2));
    }
    jx_set_word(c,0xacea,0);(void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);battle_palette(c,0x61de);
    (void)JX_CALL0(c,0x1f2a,0xc24e);return result;
}
