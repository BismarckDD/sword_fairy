/* High-level reconstruction of all entries in overlays 006,011,014,017.
 * Original instruction evidence is in analysis/cfg/functions.
 */
#include "event_engine.h"
#include <string.h>

static uint32_t event_money(JxEventContext *c) {
    return (uint32_t)jx_word(c,0x8756)|((uint32_t)jx_word(c,0x8758)<<16);
}
static int can_afford(JxEventContext *c,uint16_t cost) {
    uint32_t money=event_money(c);return money<0x80000000u && money>=cost;
}
static uint16_t choose(JxEventContext *c,uint16_t first,uint16_t second) {
    return (uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,first,c->data_segment,second,c->data_segment);
}
static void wait_engine(JxEventContext *c) { while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {} }

void jx_event_006_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    (void)JX_CALL(c,0x2f28,0x56ec,0x55,4);jx_set_word(c,0x875c,0xffff);
    (void)JX_CALL0(c,0x2f28,0x56cc);d[0x667a]=1;
    memcpy(d+0x8988,d+0x876c,6);jx_event_refresh(c);
    (void)JX_CALL(c,0x2f28,0x944c,0x0eea,c->data_segment,2);
    (void)JX_CALL(c,0x1f2a,0xcf6a,2,47,16);
    (void)JX_CALL(c,0x1f2a,0xcf6a,2,(uint16_t)(jx_word(c,0x8760)-1),(uint16_t)(jx_word(c,0x8762)+1));
    (void)JX_CALL(c,0x1f2a,0xcd1c,2,0x0ef0,c->data_segment,1);
    (void)JX_CALL0(c,0x2f28,0x56dc);jx_event_text(c,0,0x59ee,1);
    (void)JX_CALL(c,0x1f2a,0xdb46,0);jx_event_text(c,0,0x59f2,1);jx_event_text(c,2,0x59f6,1);
    jx_event_transition_begin(c);
    d[0x6678]=0;d[0x6679]=1;d[0x667b]=1;d[0x667a]=0;d[0x667c]=1;
    d[0x667d]=0;d[0x667e]=0;d[0x6661]=0;d[0x6696]=1;d[0x3466]=0;d[0xb68a]=5;
    jx_event_refresh(c);jx_event_transition_end(c);d[0xb6c1]=1;
}
void jx_event_006_0132(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    int sum=jx_signed_byte(d[0xb6a5])+jx_signed_byte(d[0xb6a7])+jx_signed_byte(d[0xb6a8]);
    if(sum>9) d[0x3466]=1;
}

void jx_event_011_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);d[0x66f3]=0;d[0x66f4]=0;d[0x6700]=1;d[0x6701]=1;
}
void jx_event_011_001e(JxEventContext *c,int16_t npc) {
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x0f05,c->data_segment,2);
    jx_ds(c)[(uint16_t)(0x670e + npc)]=0;jx_event_refresh(c);
}
void jx_event_011_004c(JxEventContext *c,int16_t npc) {
    int state=jx_signed_byte(jx_ds(c)[(uint16_t)(0xb73a + npc)]);
    uint16_t entry=(uint16_t)(0x889c + (uint16_t)npc*86u+0x2c + 2*state);
    uint16_t text=jx_word(c,entry);
    jx_event_text(c,npc,(uint16_t)(0x59d6 + 4u*text),1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,1,0,1)==1) wait_engine(c);
    else {jx_ds(c)[(uint16_t)(0x670e + npc)]=0;jx_event_refresh(c);}
}
void jx_event_011_00c8(JxEventContext *c,int16_t npc) {
    (void)npc;
    (void)JX_CALL(c,0x1f2a,0x5fd8,13,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
    jx_event_text(c,0,0x5cda,1);jx_set_word(c,0x889a,5);
    if(jx_signed_word(jx_word(c,0x31b4))>0 && jx_signed_word(jx_word(c,0x050c))>0) {
        uint16_t music=jx_word(c,0x889a);
        uint16_t resource=jx_far_word(c,jx_word(c,0x23ae),(uint16_t)(0x510 + 2*music));
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+music)&255),0,resource);
    }
    jx_ds(c)[0xb757]=1;
}
void jx_event_011_014e(JxEventContext *c,int16_t npc) {
    (void)npc;jx_event_text(c,0,0x5ce2,1);jx_ds(c)[0xb756]=1;
}

void jx_event_014_0000(JxEventContext *c,int16_t npc) {
    if(choose(c,0x0f12,0x0f0d)!=0 || !can_afford(c,10)) return;
    uint16_t kind=jx_word(c,(uint16_t)(0x889c + 86u*(uint16_t)npc+12));
    (void)JX_CALL(c,0x1f2a,0xb6a0,1,10,kind);
}
void jx_event_014_0064(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);if(d[(uint16_t)(0xb794 + npc)]!=5) return;
    if(choose(c,0x0f1a,0x0f15)!=0) {
        if(!can_afford(c,100)) {jx_event_literal(c,npc,0x0f1f,0);return;}
        uint16_t kind=jx_word(c,(uint16_t)(0x889c + 86u*(uint16_t)npc+12));
        if((uint16_t)JX_CALL(c,0x1f2a,0xb6a0,100,100,kind)==0) return;
        jx_event_text(c,npc,0x5ba6,1);jx_event_grant(c,0x14,1);
    } else {
        if((uint16_t)JX_CALL(c,0x1f2a,0x4934,4,0,0xffff)==1) {wait_engine(c);return;}
        jx_event_text(c,npc,0x5baa,1);jx_event_text(c,npc,0x5f82,1);jx_event_grant(c,0x14,1);
    }
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x0f2e,c->data_segment,2);
    d[(uint16_t)(0x6768 + npc)]=0;jx_event_refresh(c);
}

void jx_event_017_0000(JxEventContext *c,int16_t npc) {
    if(choose(c,0x0f57,0x0f52)!=0 || !can_afford(c,10)) return;
    uint32_t money=event_money(c)-10;
    jx_set_word(c,0x8756,(uint16_t)money);jx_set_word(c,0x8758,(uint16_t)(money>>16));
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x0f5a,c->data_segment,2);
    jx_event_text(c,npc,0x5c4a,1);jx_ds(c)[(uint16_t)(0x67c2 + npc)]=0;jx_ds(c)[0x67c6]=1;
}
void jx_event_017_008c(JxEventContext *c,int16_t npc) {
    jx_event_position(c,0x8760,41,84,3);jx_set_word(c,0x9334,18);jx_set_word(c,0x875c,0xffff);
    jx_event_text(c,npc,0x5dc2,1);jx_set_word(c,0x889a,0xffff);
    (void)JX_CALL0(c,0x1832,0x22ce);jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
}
void jx_event_017_00f4(JxEventContext *c,int16_t npc) {
    (void)npc;jx_event_text(c,3,0x5cae,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,3,0x0f68,c->data_segment,2);
    uint8_t *d=jx_ds(c);d[0x67c5]=0;d[0x69a2]=0;d[0x68d1]=1;d[0x347d]=1;d[0xb80f]=5;
    jx_event_refresh(c);
}
