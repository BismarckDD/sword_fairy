#include "event_engine.h"

static int battle_ends_event(JxEventContext *c,uint16_t battle) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,battle,0,4)!=1) return 0;
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
    return 1;
}

void jx_event_178_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    (void)JX_CALL(c,0x2f28,0x944c,0x1201,c->data_segment,3);
    for(int flash=0;flash<6;flash++) {
        jx_set_word(c,0x875a,0xffff); jx_set_word(c,0x875c,0xffff);
        jx_event_refresh(c);
        jx_set_word(c,0x875a,0); jx_set_word(c,0x875c,4);
        jx_event_refresh(c);
    }
    if((uint16_t)JX_CALL(c,0x1f2a,0x8ca8,21)!=0) {
        jx_event_text(c,1,0x5a96,1);
        (void)JX_CALL(c,0x1f2a,0x8f28,21,1);
    } else {
        /* This script uses signed IDIV, unlike the general recovery helper. */
        for(unsigned actor=0;actor<=4;actor+=4) {
            uint16_t base=(uint16_t)(0xd04e + actor*68);
            jx_set_word(c,(uint16_t)(base+0x24),2);
            for(unsigned field=0x10;field<=0x12;field+=2) {
                uint16_t at=(uint16_t)(base+field);
                jx_set_word(c,at,(uint16_t)(jx_signed_word(jx_word(c,at))/2));
            }
        }
        jx_event_text(c,1,0x5a6a,1);
    }
    (void)JX_CALL(c,0x2f28,0x56ec,'U',5);
    jx_event_text(c,1,0x5a6e,1); jx_event_text(c,1,0x5a72,1);
    (void)JX_CALL(c,0x2f28,0x56ec,'D',5);
    (void)JX_CALL(c,0x1f2a,0xcc24,jx_word(c,0x89de),(uint16_t)(jx_word(c,0x89e0)+1));
    (void)JX_CALL(c,0x2f28,0x944c,0x1204,c->data_segment,3);
    static const uint16_t removed[]={0x7aa3,0x7aa2,0x7aa4};
    for(unsigned i=0;i<3;i++) {
        if(battle_ends_event(c,27)) return;
        jx_ds(c)[removed[i]]=0; jx_event_refresh(c);
    }
    jx_event_text(c,7,0x5a76,1);
    if(battle_ends_event(c,22)) return;
    jx_event_text(c,0,0x5a82,1);
    if(battle_ends_event(c,21)) return;
    jx_ds(c)[0x7aa0]=0; jx_ds(c)[0x7aa1]=0; jx_ds(c)[0x7aa7]=1;
    jx_event_refresh(c);
    (void)JX_CALL(c,0x1f2a,0xdb46,7);
    jx_event_text(c,7,0x5a7a,1);
    jx_ds(c)[0x7aa7]=0; jx_ds(c)[0x7aa8]=1;
    jx_event_refresh(c);
    (void)JX_CALL(c,0x1f2a,0xdb46,8);
    jx_event_text(c,0,0x5a7e,1); jx_event_text(c,0,0x5a86,1);
    jx_ds(c)[0x7aa8]=0;
    jx_event_transition_begin(c);
    jx_event_music(c,7,2);
    jx_event_refresh(c);
    jx_event_transition_end(c);
    jx_set_word(c,0xd072,0); jx_set_word(c,0xd182,0);
}
