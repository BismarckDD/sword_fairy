/* Hand-reconstructed structured C, not an instruction interpreter.
 * Evidence: analysis/cfg/functions/ovl_NNN_OOOO.asm.
 * Names identify original module and entry. Unknown engine calls preserve ABI.
 */
#include "event_engine.h"

void jx_event_015_0000(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);
    if (jx_signed_byte(d[0xb7b2])>=5) {
        d[0xb7b2]=4;d[0x347d]=1;
        return;
    }
    if ((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x0f40,c->data_segment,0x0f3b,c->data_segment)!=0) return;
    uint32_t money=(uint32_t)jx_word(c,0x8756)|((uint32_t)jx_word(c,0x8758)<<16);
    if (money>=0x80000000u || money<10) { jx_event_literal(c,npc,0x0f43,0);return; }
    money-=10;jx_set_word(c,0x8756,(uint16_t)money);jx_set_word(c,0x8758,(uint16_t)(money>>16));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875a));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875c));
    jx_event_position(c,0x8760,31,18,1);jx_set_word(c,0x9334,15);
    jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    if(jx_word(c,0x875c)==1) jx_set_word(c,0x8752,(uint16_t)(jx_word(c,0x8752)+5));
}

void jx_event_018_0000(JxEventContext *c,int16_t npc) {
    jx_event_text(c,npc,0x5b5e,1);
    uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x0f76,c->data_segment,0x0f73,c->data_segment);
    if(choice!=0) {
        jx_event_text(c,npc,0x5b62,1);
        jx_ds(c)[(uint16_t)(0xb80c + npc)]=4;
    } else {
        jx_event_text(c,npc,0x5b6e,1);jx_event_text(c,npc,0x5b6a,1);
        jx_ds(c)[(uint16_t)(0xb80c + npc)]=4;
        jx_event_grant(c,0x15,1);
        jx_set_word(c,0x8752,(uint16_t)(jx_word(c,0x8752)+10));
    }
}
void jx_event_018_00aa(JxEventContext *c,int16_t npc) {
    if(jx_signed_byte(jx_ds(c)[(uint16_t)(0xb80c + npc)])<5) {
        jx_ds(c)[0xb80f]=6;jx_event_grant(c,0x13,1);
    }
}

void jx_event_022_0000(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);
    if(d[(uint16_t)(0xb884 + npc)]!=5) {d[0xb725]=5;return;}
    (void)JX_CALL(c,0x1f2a,0xcd1c,0,0x0f87,c->data_segment,2);
    d[0x6858]=0;d[0x67c4]=1;jx_event_refresh(c);
    jx_event_text(c,0,0x5fae,0);
}

void jx_event_031_0000(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);d[(uint16_t)(0x6966 + npc)]=0;d[0x696a]=1;
    jx_event_refresh(c);jx_event_grant(c,0x39,1);
}
void jx_event_031_002c(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);
    if(d[(uint16_t)(0xb992 + npc)]!=5) return;
    uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x0fd7,c->data_segment,0x0fd4,c->data_segment);
    if(choice!=0) {
        jx_event_text(c,npc,0x603e,1);d[(uint16_t)(0xb992 + npc)]=7;d[0x6969]=1;
    } else {
        jx_event_text(c,npc,0x6036,1);jx_event_text(c,0,0x603a,1);
        jx_event_transition_begin(c);(void)JX_CALL0(c,0x1f2a,0xb196);
    }
}

void jx_event_033_0000(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);uint16_t address=(uint16_t)(0xb9ce + npc);
    int state=jx_signed_byte(d[address]);d[0x346b]=(uint8_t)(state>=3);d[0xb830]=5;
    if(state==5) {
        jx_event_text(c,npc,0x6022,1);jx_event_text(c,npc,0x6026,1);
        jx_event_grant(c,0x34,1);jx_event_grant(c,0x30,1);d[address]=6;
    }
}

void jx_event_043_0000(JxEventContext *c,int16_t npc) {
    uint16_t kind=jx_word(c,(uint16_t)(0x889c + (uint16_t)npc*86u+14));
    uint16_t accepted=(uint16_t)JX_CALL(c,0x1f2a,0x4934,kind==0x3e ? 10 : 9,1,0);
    if(accepted==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) { /* wait for engine */ }
    } else {
        jx_ds(c)[(uint16_t)(0x6ace + npc)]=0;jx_event_refresh(c);
    }
}

void jx_event_058_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    jx_event_position(c,0x8760,25,67,1);jx_event_position(c,0x876c,26,67,1);
    jx_event_position(c,0x8778,24,68,3);jx_set_word(c,0x9334,56);
    jx_event_transition_begin(c);(void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_set_word(c,0x889a,5);
    if(jx_signed_word(jx_word(c,0x31b4))>0 && jx_signed_word(jx_word(c,0x050c))>0) {
        uint16_t music=jx_word(c,0x889a);
        uint16_t resource=jx_far_word(c,jx_word(c,0x23ae),(uint16_t)(0x510 + music*2));
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+music)&255),2,resource);
    }
    jx_event_text(c,0,0x5bca,1);jx_ds(c)[0x6cea]=0;jx_set_word(c,0x875e,0xffff);
    jx_event_transition_begin(c);jx_event_refresh(c);jx_event_transition_end(c);
    jx_event_text(c,0,0x5bce,1);
}

void jx_event_111_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    for(uint16_t i=0;i<3;++i) {
        uint16_t slot=(uint16_t)(0x64e4 + 8*i);
        jx_set_word(c,slot,(uint16_t)(28*i));jx_set_word(c,(uint16_t)(slot+2),1);
        jx_set_word(c,(uint16_t)(slot+4),(uint16_t)(0x5dd + i));
    }
    (void)JX_CALL(c,0x2f28,0x7062,0x10c7,c->data_segment,7,1);
    jx_event_refresh(c);jx_event_transition_end(c);
}

void jx_event_123_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);if(d[0xc477]!=0) return;
    (void)JX_CALL(c,0x1f2a,0xcc24,jx_word(c,0x88dc),(uint16_t)(jx_word(c,0x88de)+3));
    (void)JX_CALL(c,0x2f28,0x944c,0x10d3,c->data_segment,2);
    jx_event_text(c,0,0x5b6a,1);jx_event_text(c,0,0x5b6e,1);
    jx_set_word(c,0x875c,0xffff);d[0x742e]=0;
    jx_event_transition_begin(c);jx_event_refresh(c);jx_event_transition_end(c);d[0xc477]=1;
}

void jx_event_156_0000(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);if(d[0x780c]!=0) return;
    jx_event_text(c,npc,0x5ad2,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x1146,c->data_segment,3);
    (void)JX_CALL(c,0x1f2a,0x5fd8,0x4c,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
    (void)JX_CALL(c,0x1f2a,0xdb46,2);(void)JX_CALL(c,0x1f2a,0x2dbc,20);
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x1148,c->data_segment,3);
    (void)JX_CALL(c,0x1f2a,0xcc24,25,5);
    jx_event_text(c,npc,0x5abe,1);jx_event_text(c,npc,0x5ada,1);
    jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x875e,0xffff);jx_event_transition_begin(c);
    jx_event_position(c,0x8760,11,59,0);jx_set_word(c,0x9334,152);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);d[0x34fd]=0;
}

