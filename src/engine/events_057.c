#include "event_engine.h"

void jx_event_057_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_set_word(c,0x889a,0xffff);
    if(jx_signed_word(jx_word(c,0x31b4))>0 && jx_signed_word(jx_word(c,0x050c))>0)
        (void)JX_CALL0(c,0x1832,0x22f2);
    (void)JX_CALL(c,0x2f28,0x56ec,'U',4);
    (void)JX_CALL(c,0x2f28,0x56ec,'L',3);
    jx_event_text(c,0,0x5b6a,1);
    static const uint16_t times[]={0,14,30,46,60};
    for(unsigned i=0;i<5;i++) jx_event_position(c,(uint16_t)(0x64e4 + i*8),times[i],1,(uint16_t)(1401+i));
    (void)JX_CALL(c,0x2f28,0x7062,0x1002,c->data_segment,7,1);
    jx_event_refresh(c); jx_event_transition_end(c);
    jx_event_text(c,0,0x5b6e,1);
    (void)JX_CALL(c,0x2f28,0x944c,0x100e,c->data_segment,2);
    (void)JX_CALL(c,0x1f2a,0xcc24,(uint16_t)(jx_word(c,0x8932)+1),(uint16_t)(jx_word(c,0x8934)+1));
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,8,0,1)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    jx_event_text(c,1,0x5b72,1); jx_ds(c)[0x6c73]=0;
    jx_event_refresh(c); jx_event_music(c,10,2);
    (void)JX_CALL(c,0x1f2a,0x2dbc,18);
    jx_ds(c)[0x6c72]=0; jx_ds(c)[0x6c75]=1;
    jx_event_refresh(c);
    (void)JX_CALL(c,0x1f2a,0x2dbc,18);
    jx_event_transition_begin(c);
    jx_ds(c)[0x6c75]=0; jx_ds(c)[0x6c76]=1;
    jx_event_refresh(c);
    (void)JX_CALL(c,0x2f28,0x944c,0x1012,c->data_segment,2);
    jx_event_transition_end(c);
    jx_event_text(c,0,0x5b76,1); jx_event_text(c,0,0x5b7a,1);
    uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x1019,c->data_segment,0x1014,c->data_segment);
    if(choice!=0) {
        static const uint16_t text[]={0x5b7e,0x5b9a,0x5b9e,0x5ba2,0x5ba6};
        for(unsigned i=0;i<5;i++) jx_event_text(c,0,text[i],1);
        (void)JX_CALL(c,0x2f28,0x6f12,0x101c,c->data_segment,300,0);
        jx_event_text(c,2,0x5bae,1); jx_event_text(c,2,0x5bb2,1);
    } else {
        jx_event_text(c,0,0x5b82,1);
        (void)JX_CALL(c,0x2f28,0x6f12,0x1028,c->data_segment,300,0);
        jx_event_text(c,2,0x5b92,1);
        static const uint16_t text[]={0x5b96,0x5b9a,0x5b9e,0x5ba6};
        for(unsigned i=0;i<4;i++) jx_event_text(c,0,text[i],1);
        jx_event_text(c,2,0x5baa,1); jx_event_text(c,2,0x5bae,1);
        jx_ds(c)[0xcd40]=1;
    }
    jx_event_transition_begin(c); jx_event_music(c,9,2);
    jx_ds(c)[0x6c76]=0; jx_set_word(c,0x875e,15);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);
    jx_event_refresh(c); jx_event_transition_end(c);
    jx_ds(c)[0xbcba]=1;
}
