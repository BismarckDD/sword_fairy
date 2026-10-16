#include "event_engine.h"

void jx_event_130_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_ds(c)[0x7500]=1; jx_ds(c)[0x7501]=1;
    for(unsigned i=0;i<2;i++) {
        uint16_t base=(uint16_t)(0x5126 + i*8);
        jx_set_word(c,base,(uint16_t)i); jx_set_word(c,(uint16_t)(base+2),1);
        jx_set_word(c,(uint16_t)(base+4),(uint16_t)(0x10d5 + i*16));
        jx_set_word(c,(uint16_t)(base+6),c->data_segment);
    }
    (void)JX_CALL0(c,0x2f28,0x95aa);
    (void)JX_CALL(c,0x2f28,0x944c,0x10f5,c->data_segment,2);
    jx_event_text(c,0,0x5c82,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,0,0x10f7,c->data_segment,2);
    jx_ds(c)[0x7500]=0;
    jx_event_transition_begin(c);
    jx_event_position(c,0x8760,84,42,1); jx_event_position(c,0x876c,86,42,1);
    jx_ds(c)[0x74e8]=1; jx_set_word(c,0x9334,124);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    (void)JX_CALL(c,0x1f2a,0x72b2,18);
    jx_event_music(c,15,2);
    int battle=jx_signed_word((uint16_t)JX_CALL(c,0x1f2a,0x4934,15,0,1));
    if(battle>=0) {
        if(battle==0) jx_event_text(c,6,0x5c8e,0);
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    jx_event_text(c,6,0x5c86,1);
    for(unsigned i=0;i<8;i++) jx_event_position(c,(uint16_t)(0x64e4 + i*8),(uint16_t)(37*i),1,(uint16_t)(1901+i));
    (void)JX_CALL(c,0x2f28,0x7062,0x1101,c->data_segment,7,1);
    jx_set_word(c,0x8aae,228);
    for(unsigned i=0;i<6;i++) jx_ds(c)[0x8ae0 + i]=jx_ds(c)[0x8760 + i];
    jx_set_word(c,0x8aba,1); jx_set_word(c,0x875a,1); jx_set_word(c,0x875c,0xffff);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);
    jx_event_refresh(c); jx_event_transition_end(c);
    jx_event_grant(c,10,1);
    (void)JX_CALL(c,0x2f28,0x7062,0x110d,c->data_segment,7,0);
    jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x8f28,23,3);
    jx_event_music(c,10,2);
    jx_event_position(c,0x8760,2,9,3);
    (void)JX_CALL(c,0x1f2a,0x747c,1);
    (void)JX_CALL(c,0x1f2a,0x8ec4,30,0);
    jx_set_word(c,0x9334,108); jx_ds(c)[0x7303]=1; jx_ds(c)[0x7304]=1;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_event_text(c,0,0x5caa,1); jx_event_text(c,0,0x5cae,1);
    for(unsigned i=0;i<3;i++) jx_event_text(c,1,(uint16_t)(0x5cb6 + i*4),1);
    jx_event_text(c,0,0x5cca,1);
    uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x111f,c->data_segment,0x1118,c->data_segment);
    if(choice!=0) {
        jx_event_position(c,0x8760,23,92,1);
        jx_set_word(c,0x9334,129); jx_set_word(c,0x8894,4);
        (void)JX_CALL0(c,0x1f2a,0x55f2);
    } else {
        jx_event_position(c,0x8760,9,65,1);
        jx_set_word(c,0x8894,4);
        (void)JX_CALL0(c,0x1f2a,0x55f2);
        jx_set_word(c,0x9334,138);
    }
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_event_music(c,5,2);
}
