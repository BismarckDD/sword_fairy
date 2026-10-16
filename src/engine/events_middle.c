#include "event_engine.h"

void jx_event_037_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);if(d[0xba48]!=0) return;
    for(unsigned i=0;i<10;++i) d[0x6ace + i]=1;
    jx_event_grant(c,23,3);d[0x348f]=1;d[0xba62]=2;d[0xbaa0]=5;
    d[0x6a75]=0;d[0x6a76]=0;d[0x6a77]=0;
}

void jx_event_037_0064(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);jx_event_music(c,6,2);jx_event_text(c,0,0x5af2,1);
    (void)JX_CALL(c,0x1f2a,0xcc24,23,4);(void)JX_CALL(c,0x1f2a,0xcc24,26,4);
    jx_event_position(c,0x64e4,0,0,72);jx_set_word(c,0x64ea,0);jx_event_position(c,0x64ec,5,1,48);
    (void)JX_CALL(c,0x2f28,0x7062,0x0ff2,c->data_segment,2,1);
    d[0x6a1a]=1;d[0x6a1b]=1;jx_event_refresh(c);jx_event_transition_end(c);
    (void)JX_CALL(c,0x1f2a,0xcc24,23,4);
    (void)JX_CALL(c,0x1f2a,0xcc24,jx_word(c,0x8932),(uint16_t)(jx_word(c,0x8934)-1));
    (void)JX_CALL(c,0x1f2a,0xcd1c,1,0x0ffe,c->data_segment,2);
    (void)JX_CALL(c,0x2f28,0x944c,0x1000,c->data_segment,2);
    jx_event_text(c,1,0x5afa,1);jx_event_text(c,1,0x5afe,1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,4,1,0)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    d[0x6a1b]=0;jx_event_refresh(c);jx_event_text(c,0,0x5b02,1);
    d[0x3485]=0;d[0x348f]=0;d[0xba63]=1;
}

void jx_event_083_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    if(jx_signed_word(jx_word(c,0x875e))<0) {
        jx_event_text(c,0,0x5c3a,1);(void)JX_CALL(c,0x1f2a,0xdb46,0);jx_event_text(c,0,0x5c3e,1);
        jx_event_transition_begin(c);jx_set_word(c,0x875a,2);jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x875e,0xffff);
        d[0x6f7e]=0;d[0x6f7f]=1;d[0x6f80]=1;d[0xbf8c]=1;
        (void)JX_CALL0(c,0x1f2a,0xc6a8);jx_set_word(c,0xd0fa,1);jx_set_word(c,0x5860,3);
        jx_event_refresh(c);jx_event_music(c,4,2);jx_event_transition_end(c);
    } else {
        (void)JX_CALL(c,0x1f2a,0xdb46,0);
        jx_event_text(c,0,0x5c8a,1);jx_event_text(c,0,0x5c8e,1);jx_event_text(c,0,0x5c92,1);
        jx_event_transition_begin(c);d[0x6f7e]=0;d[0x6d62]=1;jx_set_word(c,0x875e,0xffff);
        jx_event_refresh(c);jx_event_transition_end(c);jx_event_music(c,5,2);
    }
}

static int battle_returns_to_wait(JxEventContext *c,uint16_t a,uint16_t b,uint16_t d) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,a,b,d)!=1) return 0;
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
    return 1;
}
static void sequence_entry(JxEventContext *c,uint16_t slot,uint16_t a,uint16_t b,uint16_t d) {
    jx_set_word(c,slot,a);jx_set_word(c,(uint16_t)(slot+2),b);jx_set_word(c,(uint16_t)(slot+4),d);
}

void jx_event_035_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    sequence_entry(c,0x64e4,0,1,45);sequence_entry(c,0x64ec,5,1,46);
    (void)JX_CALL(c,0x2f28,0x7062,0x0fda,c->data_segment,3,0);
    jx_event_text(c,0,0x59de,1);jx_set_word(c,0x875c,0xffff);d[0x69de]=1;
    jx_event_position(c,0x8760,19,20,1);jx_event_refresh(c);jx_event_text(c,0,0x59e2,1);
    sequence_entry(c,0x64e4,5,1,47);sequence_entry(c,0x64ec,9,0,4);jx_set_word(c,0x64f2,0);
    (void)JX_CALL(c,0x2f28,0x7062,0x0fe6,c->data_segment,3,0);
    (void)JX_CALL(c,0x1f2a,0x2dbc,540);
    d[0xba27]=1;d[0x69de]=0;d[0x69df]=1;jx_event_refresh(c);
}
void jx_event_035_00e8(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);d[(uint16_t)(0x69de + npc)]=0;
    jx_set_word(c,0x875a,1);jx_set_word(c,0x875c,0);jx_set_word(c,0x875e,0xffff);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);jx_event_grant(c,0x12,1);
    d[0xba28]=5;d[0xba29]=5;jx_event_refresh(c);
}

void jx_event_046_0000(JxEventContext *c,int16_t npc) {
    (void)npc;if(battle_returns_to_wait(c,9,1,0)) return;
    jx_ds(c)[0x6b28]=0;jx_event_refresh(c);
    if(battle_returns_to_wait(c,10,1,0)) return;
    jx_ds(c)[0x6b29]=0;jx_event_refresh(c);
    jx_event_position(c,0x8760,22,66,0);jx_event_position(c,0x876c,22,67,0);
    jx_set_word(c,0x9334,45);jx_ds(c)[0x6c36]=1;jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
}
void jx_event_046_00be(JxEventContext *c,int16_t npc) {
    uint16_t kind=jx_word(c,(uint16_t)(0x889c + (uint16_t)npc*86u+14));
    if(battle_returns_to_wait(c,kind==0x3e ? 10 : 9,1,0)) return;
    jx_ds(c)[(uint16_t)(0x6b28 + npc)]=0;jx_event_refresh(c);
}

void jx_event_086_0000(JxEventContext *c,int16_t npc) {
    (void)npc;if(battle_returns_to_wait(c,11,2,0xffff)) return;
    uint8_t *d=jx_ds(c);d[0x6fe0]=1;d[0x6fd9]=0;jx_set_word(c,0x875a,0xffff);
    jx_event_refresh(c);(void)JX_CALL(c,0x1f2a,0xdb46,8);jx_event_text(c,1,0x5c6a,1);
    d[0x6fdb]=1;d[0x6f7f]=0;d[0x6f80]=0;
    (void)JX_CALL(c,0x1f2a,0xcd1c,3,0x1065,c->data_segment,2);
    jx_set_word(c,0x875a,0);jx_set_word(c,0x875c,1);jx_set_word(c,0x875e,0xffff);
    jx_event_text(c,1,0x5c6e,0);jx_set_word(c,0x875e,7);
    jx_event_position(c,0x8760,65,115,3);jx_event_position(c,0x876c,63,115,3);jx_event_position(c,0x8778,61,115,3);
    jx_set_word(c,0x9334,70);jx_set_word(c,0xd0fa,0);
    d[0x6fd8]=0;d[0x6fda]=0;d[0x6fdb]=0;d[0x6fe0]=0;jx_set_word(c,0x5860,1);
    jx_event_transition_begin(c);(void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
}
void jx_event_086_0130(JxEventContext *c,int16_t npc) {
    (void)npc;(void)JX_CALL(c,0x2f28,0x56ec,'U',5);jx_event_text(c,1,0x5d0a,0);
    (void)JX_CALL(c,0x1f2a,0xdb46,9);(void)JX_CALL(c,0x2f28,0x56ec,'D',5);
    uint8_t *d=jx_ds(c);d[0x6fe1]=0;d[0x6fd8]=1;d[0x6fd9]=1;
}
