#include "event_engine.h"

static void play_effect(JxEventContext *c,uint16_t effect) {
    (void)JX_CALL(c,0x1f2a,0x5fd8,effect,6);
    (void)JX_CALL(c,0x1f2a,0x155c,6);
}

void jx_event_027_0000(JxEventContext *c,int16_t npc) {
    play_effect(c,20);
    jx_set_word(c,(uint16_t)(0x889c + npc*86 + 0x3c),1);
    (void)JX_CALL(c,0x1f2a,0xca66,(uint16_t)npc);
    play_effect(c,19);
    (void)JX_CALL(c,0x1f2a,0xdb46,(uint16_t)(npc+1));
    jx_ds(c)[(uint16_t)(0x68ee + npc+1)]=0;
    jx_event_refresh(c);
}

/* Four switches form a decimal code; arithmetic wraps at 16 bits. */
void jx_event_027_0072(JxEventContext *c,int16_t npc) {
    uint16_t frame=(uint16_t)(0x889c + npc*86 + 0x3c);
    jx_set_word(c,frame,jx_word(c,frame)==0 ? 1 : 0);
    play_effect(c,20);
    (void)JX_CALL(c,0x1f2a,0xca66,(uint16_t)npc);
    uint16_t code=(uint16_t)(1000u*jx_word(c,0x8c34)+100u*jx_word(c,0x8c8a)
        +10u*jx_word(c,0x8ce0)+jx_word(c,0x8d36));
    if(code==1011) {
        play_effect(c,19);
        (void)JX_CALL(c,0x1f2a,0xdb46,14);
        jx_ds(c)[0x68fc]=0;
    }
}

void jx_event_027_0124(JxEventContext *c,int16_t npc) {
    (void)npc;
    int member=jx_signed_word(jx_word(c,0x875c));
    uint8_t gate=jx_ds(c)[0x68fc];
    if(gate!=0 && member>0) {
        jx_event_text(c,0,0x5db2,1);
        jx_set_word(c,0x875c,0xffff);
        jx_event_refresh(c);
        (void)JX_CALL(c,0x2f28,0x944c,0x0fcc,c->data_segment,3);
    } else if(gate==0 && member<0) {
        jx_set_word(c,0x875c,1); jx_ds(c)[0x346c]=1;
        (void)JX_CALL0(c,0x1f2a,0xc6a8);
        jx_event_refresh(c);
        jx_event_text(c,0,0x5dba,1);
    } else if(gate==1 && member<0) {
        (void)JX_CALL(c,0x2f28,0x944c,0x0fcf,c->data_segment,3);
    }
}

void jx_event_027_01e4(JxEventContext *c,int16_t npc) {
    jx_event_transition_begin(c);
    jx_ds(c)[(uint16_t)(0x68ee + npc)]=0;
    jx_event_refresh(c);
    jx_event_transition_end(c);
    jx_event_text(c,0,0x5d0a,1);
    jx_set_word(c,0x8894,2);
    (void)JX_CALL0(c,0x1f2a,0x55f2);
    jx_event_position(c,0x8760,8,16,1);
    jx_event_position(c,0x876c,8,14,1);
    jx_set_word(c,0x9334,39);
    jx_event_transition_begin(c);
    jx_event_music(c,5,2);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
}

void jx_event_027_02ca(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_event_text(c,21,0x5d02,1);
    (void)JX_CALL(c,0x1f2a,0xcf6a,21,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)-1));
    (void)JX_CALL(c,0x1f2a,0xcd1c,21,0x0fd2,c->data_segment,2);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,7,0,1)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
    } else {
        jx_ds(c)[0x6903]=0;
        jx_event_refresh(c);
    }
}
