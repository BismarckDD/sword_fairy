#include "event_engine.h"

/* Overlay 010. Offsets refer to relocated overlay code, not file offsets. */
void jx_event_010_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_event_music(c,4,0);
    jx_event_text(c,0,0x5a46,1);
    jx_ds(c)[0xb739]=1;
}

void jx_event_010_0066(JxEventContext *c,int16_t npc) {
    (void)npc;
    int total=0;
    for(int i=0;i<25;i++) total+=jx_signed_byte(jx_ds(c)[0xb71c + i]);
    if(total>=40 && jx_ds(c)[0xb847]==0) {
        jx_ds(c)[0x67fe]=1; jx_ds(c)[0x6800]=1; jx_ds(c)[0x6803]=1;
    }
}

void jx_event_010_00c0(JxEventContext *c,int16_t npc) {
    uint16_t state=(uint16_t)(0xb71c + npc);
    if(jx_ds(c)[state]==5) {
        jx_ds(c)[0x67e3]=1;
        jx_ds(c)[state]=7;
    }
}

void jx_event_010_00ee(JxEventContext *c,int16_t npc) {
    uint16_t text=jx_word(c,(uint16_t)(0x889c + npc*86 + 0x38));
    jx_event_text(c,npc,(uint16_t)(0x59d6 + text*4u),1);
    jx_ds(c)[(uint16_t)(0x66f0 + npc)]=0;
    jx_ds(c)[0xb884]=5;
    jx_set_word(c,0x875c,6);
    (void)JX_CALL(c,0x1f2a,0x8f28,0x13,1);
    uint16_t actor=jx_word(c,0x875c);
    (void)JX_CALL(c,0x1f2a,0xc264,0x1f,jx_word(c,(uint16_t)(0xd04e + actor*68u + 14)));
    jx_ds(c)[0xb7ef]=4;
    jx_event_refresh(c);
    jx_event_music(c,13,0);
}

void jx_event_010_01ac(JxEventContext *c,int16_t npc) {
    if(jx_ds(c)[(uint16_t)(0xb71c + npc)]!=5) return;
    uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x0ef7,c->data_segment,0x0ef2,c->data_segment);
    if(choice==0) {
        jx_event_text(c,npc,0x5c2e,1);
        jx_event_text(c,npc,0x5c32,1);
        jx_ds(c)[0xb72b]=6;
    }
}

void jx_event_010_0226(JxEventContext *c,int16_t npc) {
    jx_event_text(c,npc,0x5a5a,1);
    jx_event_transition_begin(c);
    for(int i=0;i<4;i++) jx_ds(c)[0x6702 + i]=0;
    (void)JX_CALL(c,0x1f2a,0x8f28,20,1);
    jx_event_refresh(c);
    jx_event_transition_end(c);
}

void jx_event_010_028c(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_event_position(c,0x64e4,0,1,10);
    (void)JX_CALL(c,0x2f28,0x7062,0x0efa,c->data_segment,3,0);
    jx_event_position(c,0x8760,56,31,0);
    jx_set_word(c,0x9334,13);
    jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_set_word(c,0x8a34,(uint16_t)(jx_word(c,0x8a34)-2));
    jx_event_refresh(c);
}
