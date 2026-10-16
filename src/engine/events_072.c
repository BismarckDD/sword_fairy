#include "event_engine.h"

void jx_event_072_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    if(jx_ds(c)[0xc07b]==2) {
        (void)JX_CALL(c,0x2f28,0x944c,0x1034,c->data_segment,2);
        jx_event_text(c,0,0x5d72,1);
        if((uint16_t)JX_CALL(c,0x1f2a,0x4934,12,0,1)==1) {
            while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
            return;
        }
        static const uint16_t hidden[]={0x6e34,0x6dbc,0x6dbd,0x6dbe,0x6dbf,0x6df8,0x6df9,0x7032,0x7033,0x7034};
        for(unsigned i=0;i<sizeof(hidden)/sizeof(hidden[0]);i++) jx_ds(c)[hidden[i]]=0;
        jx_set_word(c,0x875e,0xffff);
        jx_event_refresh(c);
        jx_event_text(c,1,0x5d76,1); jx_event_text(c,-1,0x5d36,1);
        jx_ds(c)[0xc07b]=3;
    } else if(jx_ds(c)[0xc07b]==0) {
        (void)JX_CALL(c,0x2f28,0x56ec,'U',10);
        (void)JX_CALL(c,0x2f28,0x56ec,'U',5);
        jx_event_text(c,0,0x5cd2,0);
        (void)JX_CALL(c,0x2f28,0x56ec,'D',5);
        (void)JX_CALL(c,0x2f28,0x56ec,'D',10);
        (void)JX_CALL(c,0x2f28,0x944c,0x1048,c->data_segment,2);
        static const int16_t speakers[]={0,0,1,0,0,0};
        for(unsigned i=0;i<6;i++) jx_event_text(c,speakers[i],(uint16_t)(0x5cd6 + i*4),1);
        (void)JX_CALL(c,0x1f2a,0xcd1c,1,0x105c,c->data_segment,2);
        jx_event_text(c,1,0x5cee,1); jx_event_text(c,0,0x5cf2,1);
        (void)JX_CALL(c,0x2f28,0x944c,0x1060,c->data_segment,2);
        jx_event_position(c,0x8760,30,19,0);
        jx_set_word(c,0x8766,30); jx_set_word(c,0x8768,18);
        jx_set_word(c,0x876c,30); jx_set_word(c,0x876e,17);
        jx_set_word(c,0x9334,84);
        jx_event_transition_begin(c);
        (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
        jx_event_text(c,0,0x5cf6,1);
        jx_ds(c)[0xbe24]=5; jx_ds(c)[0xc05e]=5; jx_ds(c)[0xc05f]=5;
        jx_ds(c)[0xc07b]=1;
        jx_ds(c)[0x6dbc]=0; jx_ds(c)[0x6dbd]=0;
        jx_ds(c)[0x6dbe]=1; jx_ds(c)[0x6dbf]=1;
    }
}
