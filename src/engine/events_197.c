#include "event_engine.h"

static void walk(JxEventContext *c,uint16_t npc,uint16_t x,uint16_t y) {
    (void)JX_CALL(c,0x1f2a,0xcf6a,npc,x,y);
}
static void gesture(JxEventContext *c,uint16_t npc,uint16_t script) {
    (void)JX_CALL(c,0x1f2a,0xcd1c,npc,script,c->data_segment,3);
}
static int defeated(JxEventContext *c,uint16_t battle) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,battle,0,0xffff)!=1) return 0;
    (void)JX_CALL0(c,0x2f28,0x56dc);
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
    return 1;
}

void jx_event_197_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_ds(c)[0x7cda]=0; jx_ds(c)[0x7cdb]=1; jx_event_refresh(c);
    jx_event_text(c,0,0x5a12,1);
    jx_ds(c)[0x7cdb]=0; jx_event_refresh(c);
    walk(c,9,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)-1));
    gesture(c,9,0x1283); jx_event_text(c,9,0x5a2a,1);
    walk(c,9,43,13); gesture(c,9,0x1285);
    (void)JX_CALL(c,0x1f2a,0xcc24,54,13);
    (void)JX_CALL(c,0x2f28,0x944c,0x1287,c->data_segment,3);
    (void)JX_CALL(c,0x2f28,0x56ec,'L',4);
    (void)JX_CALL0(c,0x2f28,0x56cc);
    static const struct {uint16_t npc,battle,text,x,y;} opponents[]={
        {8,32,0x5a46,45,9},{3,29,0x5a2e,49,9},
        {2,30,0x5a36,53,9},{6,31,0x5a3e,46,17}};
    for(unsigned i=0;i<4;i++) {
        uint16_t who=opponents[i].npc;
        walk(c,who,49,21); walk(c,who,49,18); walk(c,who,45,13);
        gesture(c,who,(uint16_t)(0x1289 + i*4));
        jx_event_text(c,(int16_t)who,opponents[i].text,1);
        if(defeated(c,opponents[i].battle)) return;
        jx_event_text(c,(int16_t)who,(uint16_t)(opponents[i].text+4),1);
        walk(c,who,opponents[i].x,opponents[i].y);
        gesture(c,who,(uint16_t)(0x128b + i*4));
    }
    walk(c,9,45,13); gesture(c,9,0x1299);
    jx_event_text(c,9,0x5a52,1);
    if(defeated(c,33)) return;
    jx_event_text(c,9,0x5a56,1); jx_event_text(c,-1,0x5a5a,1);
    if(jx_ds(c)[0xc5fe]!=5 && jx_ds(c)[0xcd40]==0) {
        jx_ds(c)[0x7cf9]=1;
        if(jx_word(c,0x8752)<10) jx_ds(c)[0x7cfa]=1;
    }
    for(unsigned i=0;i<20;i++) jx_ds(c)[0x7cda + i]=0;
    jx_event_transition_begin(c);
    (void)JX_CALL0(c,0x2f28,0x56dc);
    jx_event_refresh(c); jx_event_music(c,3,2); jx_event_transition_end(c);
    jx_event_text(c,0,0x5a5e,1); jx_ds(c)[0xcd23]=1;
}
