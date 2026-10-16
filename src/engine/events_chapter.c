#include "event_engine.h"

static int has_guard(JxEventContext *c) {
    for(unsigned i=0;i<5;++i) if(jx_ds(c)[0x6876 + i]==1) return 1;
    return 0;
}
static int wait_after_battle(JxEventContext *c,uint16_t encounter) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,encounter,0,0xffff)!=1) return 0;
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
    return 1;
}
void jx_event_023_0000(JxEventContext *c,int16_t npc) {
    jx_event_state_text(c,npc,0xb8a2);
    if(wait_after_battle(c,2)) return;
    jx_ds(c)[(uint16_t)(0x6876 + npc)]=0;jx_event_refresh(c);
    if(!has_guard(c)) jx_event_text(c,0,0x605a,0);
}
void jx_event_023_00ba(JxEventContext *c,int16_t npc) {
    (void)npc;
    if(has_guard(c)) {jx_event_text(c,0,0x6056,1);return;}
    jx_event_literal(c,-1,0x0f91,0);jx_set_word(c,0x9334,19);jx_ds(c)[0x3478]=0;
    jx_event_transition_begin(c);(void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
}
void jx_event_024_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    jx_event_position(c,0x8760,104,20,3);jx_event_position(c,0x876c,104,18,1);
    jx_event_position(c,0x8778,103,18,1);jx_set_word(c,0x9334,12);
    jx_event_text(c,0,0x5dc6,1);jx_event_transition_begin(c);jx_event_music(c,13,2);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);jx_event_text(c,2,0x5caa,1);
    d[0x67c4]=0;jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x875e,0xffff);
    d[0x67c5]=1;d[0x6968]=1;d[0x67e3]=0;d[0x67e6]=1;d[0x681c]=0;d[0x681d]=0;
    d[0xb7ee]=7;d[0xb7ef]=7;d[0xb7f2]=7;
    for(unsigned i=0;i<6;++i) d[0xb7d0 + i]=6;
    d[0xb992]=5;d[0xb993]=5;d[0x346b]=0;
    jx_event_transition_begin(c);jx_event_refresh(c);jx_event_transition_end(c);
}
void jx_event_026_0000(JxEventContext *c,int16_t npc) {
    jx_event_text(c,npc,0x5cca,0);jx_set_word(c,0x875c,1);
    (void)JX_CALL(c,0x1f2a,0xc264,31,jx_word(c,0xd0a0));
    uint8_t *d=jx_ds(c);d[(uint16_t)(0x68d0 + npc)]=0;jx_event_refresh(c);
    d[0x692c]=0;d[0x670f]=1;d[0x6710]=1;d[0x6711]=1;d[0x692a]=1;
    d[0x6713]=1;d[0x692b]=1;
    for(unsigned i=0;i<5;++i) d[0x6715 + i]=1;
    d[0x671a]=0;d[0x671c]=0;d[0x671d]=0;d[0x671e]=0;
}
void jx_event_026_009c(JxEventContext *c,int16_t npc) {
    jx_event_music(c,3,2);jx_event_text(c,1,0x5cb2,1);
    if(wait_after_battle(c,6)) return;
    jx_event_text(c,1,0x5cb6,1);jx_event_position(c,0x64e4,1,1,41);
    (void)JX_CALL(c,0x2f28,0x7062,0x0fb4,c->data_segment,2,0);
    uint8_t *d=jx_ds(c);d[0x68d3]=1;jx_event_text(c,1,0x5cba,0);jx_event_text(c,2,0x5ade,0);
    d[0x68d2]=1;jx_event_position(c,0x64e4,1,1,42);
    (void)JX_CALL(c,0x2f28,0x7062,0x0fc0,c->data_segment,2,0);
    d[(uint16_t)(0x68d0 + npc)]=0;d[0x68d3]=0;d[0x68d2]=0;
    jx_event_position(c,0x8760,31,18,1);jx_set_word(c,0x9334,15);jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);jx_event_text(c,0,0x5cc2,1);
    d[0xb7b2]=7;d[0x347d]=0;d[0x346a]=0;d[0x68d1]=0;d[0x68d0]=1;
    (void)JX_CALL(c,0x1f2a,0x8f28,0x12,1);(void)JX_CALL(c,0x1f2a,0x747c,0);
}
