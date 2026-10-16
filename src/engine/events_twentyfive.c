#include "event_engine.h"

/* Overlay 025, four independently dispatched scene events. */
void jx_event_025_0000(JxEventContext *c,int16_t npc) {
    jx_event_text(c,npc,0x5c9a,1);
    jx_set_word(c,0x875e,5);
    (void)JX_CALL(c,0x1f2a,0xc264,32,jx_word(c,0xd04e + 5*68 + 14));
    jx_ds(c)[(uint16_t)(0x68b2 + npc)]=0;
    jx_event_refresh(c);
    jx_ds(c)[0x3479]=1; jx_ds(c)[0x3478]=0; jx_ds(c)[0xb8c0]=5;
}

void jx_event_025_005c(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_event_text(c,0,0x5c82,0);
    static const uint16_t script[]={16,0,172,4,27,0,340,4};
    for(unsigned i=0;i<8;i++) jx_set_word(c,(uint16_t)(0x64e4 + i*2),script[i]);
    (void)JX_CALL(c,0x2f28,0x7062,0x0f9c,c->data_segment,5,1);
    jx_ds(c)[0x68b6]=0;
    jx_set_word(c,0x875c,1);
    (void)JX_CALL(c,0x1f2a,0xc264,31,jx_word(c,0xd04e + 68 + 14));
    jx_event_refresh(c);
    jx_event_music(c,18,2);
}

void jx_event_025_0126(JxEventContext *c,int16_t npc) {
    (void)JX_CALL(c,0x1f2a,0x5fd8,19,6);
    (void)JX_CALL(c,0x1f2a,0x155c,6);
    (void)JX_CALL(c,0x1f2a,0xdb46,(uint16_t)npc);
    jx_ds(c)[(uint16_t)(0x68b2 + npc)]=0;
    jx_event_refresh(c);
}

void jx_event_025_0162(JxEventContext *c,int16_t npc) {
    (void)npc;
    (void)JX_CALL(c,0x2f28,0x56ec,'U',6);
    (void)JX_CALL(c,0x2f28,0x56ec,'L',3);
    jx_event_text(c,0,0x5c7e,1);
    jx_event_position(c,0x64e4,0,1,43);
    jx_event_position(c,0x64ec,14,1,44);
    jx_event_position(c,0x64f4,24,0,245);
    jx_set_word(c,0x64fa,0);
    (void)JX_CALL(c,0x2f28,0x7062,0x0fa8,c->data_segment,5,0);
    jx_ds(c)[0x68b3]=0; jx_ds(c)[0x68b2]=0;
    jx_set_word(c,0x8762,(uint16_t)(jx_word(c,0x8762)-10));
    jx_set_word(c,0x8764,1); jx_ds(c)[0x68b6]=1;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,5,0,0xffff)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    (void)JX_CALL(c,0x1f2a,0x2dbc,30);
    jx_event_position(c,0x8760,48,84,2);
    jx_set_word(c,0x9334,19);
    jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_event_text(c,0,0x5c66,1);
    jx_event_position(c,0x8760,56,104,2);
    jx_set_word(c,0x9334,20);
    jx_event_transition_begin(c);
    jx_ds(c)[0x68b5]=1;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    (void)JX_CALL(c,0x1f2a,0x747c,0);
    jx_ds(c)[0x3479]=0; jx_ds(c)[0x3478]=1;
    jx_event_music(c,7,2);
}
