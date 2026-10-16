#include "event_engine.h"

static void move_npc(JxEventContext *c,uint16_t npc,uint16_t x,uint16_t y) {
    (void)JX_CALL(c,0x1f2a,0xcf6a,npc,x,y);
}
static void script_text(JxEventContext *c,uint16_t text) {
    (void)JX_CALL(c,0x2f28,0x944c,text,c->data_segment,2);
}
static void npc_script(JxEventContext *c,uint16_t npc,uint16_t text) {
    (void)JX_CALL(c,0x1f2a,0xcd1c,npc,text,c->data_segment,2);
}

/* Overlay 019: scripted sequence, retaining every engine-call boundary. */
void jx_event_019_0000(JxEventContext *c,int16_t npc) {
    jx_event_state_text(c,npc,0xb82a);
    jx_ds(c)[0x67fe]=0; jx_ds(c)[0x6802]=1;
    jx_event_refresh(c);
    move_npc(c,4,(uint16_t)(jx_word(c,0x8760)+1),(uint16_t)(jx_word(c,0x8762)+1));
    jx_event_text(c,4,0x5f06,1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,3,0,2)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    move_npc(c,2,14,18);
    jx_ds(c)[(uint16_t)(0x67fe + npc)]=0;
    jx_event_refresh(c);
    jx_event_text(c,2,0x5c06,1); script_text(c,0x0f79);
    jx_event_text(c,4,0x5f0a,1); jx_event_text(c,4,0x5c0a,1);
    jx_ds(c)[0x67ff]=1; jx_ds(c)[0x6803]=0;
    jx_event_refresh(c);
    move_npc(c,1,(uint16_t)(jx_word(c,0x8760)+2),jx_word(c,0x8762));
    npc_script(c,4,0x0f7b); script_text(c,0x0f7d);
    jx_event_text(c,1,0x5c0e,1);
    move_npc(c,1,14,18); jx_ds(c)[0x67ff]=0;
    jx_event_refresh(c);
    npc_script(c,4,0x0f7f); jx_event_text(c,4,0x5c12,1);
    move_npc(c,4,14,18); jx_ds(c)[0x6802]=0;
    jx_event_refresh(c);
    jx_event_text(c,-1,0x5c16,1); script_text(c,0x0f81);
    jx_ds(c)[0x6801]=1;
    move_npc(c,3,(uint16_t)(jx_word(c,0x8760)+1),jx_word(c,0x8762));
    npc_script(c,3,0x0f83); jx_event_text(c,3,0x5eae,1);
    script_text(c,0x0f85);
    static const int16_t speakers[]={-1,3,0,3,-1,3,0,3,3};
    for(unsigned i=0;i<9;i++) jx_event_text(c,speakers[i],(uint16_t)(0x5eb2 + 4*i),1);
    move_npc(c,3,13,18); jx_ds(c)[0x6801]=0;
    jx_event_refresh(c);
    jx_event_text(c,-1,0x5c1e,1);
    (void)JX_CALL(c,0x1f2a,0xcc24,14,16);
    jx_ds(c)[0xb795]=5; jx_ds(c)[0xb72b]=5; jx_ds(c)[0xb847]=5;
    for(int i=0;i<6;i++) jx_ds(c)[0xb7d0 + i]=5;
    for(int i=18;i<22;i++) jx_ds(c)[0xb71c + i]=5;
}

void jx_event_019_0376(JxEventContext *c,int16_t npc) {
    if(jx_ds(c)[(uint16_t)(0xb82a + npc)]==5) {
        move_npc(c,6,1,20);
        jx_ds(c)[0x6804]=0; jx_ds(c)[0x69a3]=1; jx_ds(c)[0xb9ce]=5;
        jx_set_word(c,0x8752,(uint16_t)(jx_word(c,0x8752)-5));
        jx_event_refresh(c);
    }
}
