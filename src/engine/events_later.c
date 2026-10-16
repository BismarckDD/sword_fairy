#include "event_engine.h"
#include <string.h>

void jx_event_152_0000(JxEventContext *c,int16_t npc) {
    jx_event_text(c,npc,0x5a82,1);
    uint16_t item=(uint16_t)JX_CALL(c,0x1f2a,0x8ca8,31)!=0 ? 31 : 32;
    (void)JX_CALL(c,0x1f2a,0x8f28,item,1);
    (void)JX_CALL(c,0x1f2a,0x43b2,0,100);(void)JX_CALL(c,0x1f2a,0x43b2,0,100);
    if((uint16_t)JX_CALL(c,0x1f2a,0x8ca8,32)!=0) {
        (void)JX_CALL(c,0x1f2a,0x8f28,32,1);(void)JX_CALL(c,0x1f2a,0x43b2,0,100);
    }
    (void)JX_CALL(c,0x1f2a,0x747c,0);
    (void)JX_CALL(c,0x1f2a,0xcd1c,0,0x1126,c->data_segment,3);(void)JX_CALL(c,0x1f2a,0x2dbc,30);
    (void)JX_CALL(c,0x1f2a,0xcd1c,0,0x1132,c->data_segment,3);(void)JX_CALL(c,0x1f2a,0x2dbc,20);
    (void)JX_CALL(c,0x1f2a,0xcd1c,0,0x1142,c->data_segment,3);jx_event_text(c,npc,0x5a72,1);
    (void)JX_CALL(c,0x1f2a,0xcc24,3,8);(void)JX_CALL(c,0x2f28,0x944c,0x1144,c->data_segment,3);
    (void)JX_CALL(c,0x1f2a,0xdb46,2);jx_event_text(c,2,0x5a8e,1);jx_event_text(c,npc,0x5a92,1);
    for(uint16_t who=0;who<2;++who) {
        (void)JX_CALL(c,0x1f2a,0xcf6a,who,7,13);(void)JX_CALL(c,0x1f2a,0xcf6a,who,21,13);
    }
    uint8_t *d=jx_ds(c);d[0x7794]=0;d[0x7795]=0;d[0x7798]=1;
    jx_event_text(c,2,0x5a96,1);jx_event_transition_begin(c);
    jx_set_word(c,0x875a,0);jx_set_word(c,0x875c,1);(void)JX_CALL0(c,0x1f2a,0xc6a8);
    d[0x7796]=0;jx_event_refresh(c);jx_event_transition_end(c);jx_event_text(c,2,0x5a9a,1);
    d[0x77b2]=1;d[0x34f7]=1;
}

void jx_event_101_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,14,0,1)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    for(uint16_t i=0;i<3;++i) {
        jx_event_position(c,(uint16_t)(0x64e4+i*8),(uint16_t)(i+1),0,(uint16_t)(42+i));
        jx_set_word(c,(uint16_t)(0x64ea+i*8),1);
    }
    (void)JX_CALL(c,0x2f28,0x7062,0x1075,c->data_segment,7,0);
    jx_ds(c)[0x719a]=0;jx_ds(c)[0x719b]=1;jx_event_refresh(c);
}
void jx_event_101_009e(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);if(d[0xc23d]==0) return;
    jx_event_text(c,1,0x5a96,1);jx_set_word(c,0x889a,0xffff);(void)JX_CALL0(c,0x1832,0x22f2);
    jx_event_transition_begin(c);jx_event_position(c,0x8760,27,14,3);jx_event_position(c,0x876c,27,15,3);
    jx_event_refresh(c);jx_event_transition_end(c);
    (void)JX_CALL(c,0x2f28,0x6f12,0x1080,c->data_segment,200,0);
    d[0x719b]=0;d[0x719c]=1;d[0x719d]=1;jx_event_refresh(c);
    (void)JX_CALL(c,0x1f2a,0xcc24,33,12);
    for(uint16_t i=0;i<4;++i) d[0x719e + i]=1;
    for(uint16_t i=0;i<4;++i) {
        uint16_t slot=(uint16_t)(0x5126+i*8);
        jx_set_word(c,(uint16_t)(slot+2),1);jx_set_word(c,slot,(uint16_t)(4+i));
        jx_set_word(c,(uint16_t)(slot+4),(uint16_t)(0x108b+i*6));jx_set_word(c,(uint16_t)(slot+6),c->data_segment);
    }
    (void)JX_CALL0(c,0x2f28,0x95aa);
    (void)JX_CALL(c,0x1f2a,0xcd1c,3,0x10a2,c->data_segment,5);jx_event_text(c,4,0x5aa2,1);
    d[0x719d]=0;(void)JX_CALL(c,0x2f28,0x6f12,0x10a7,c->data_segment,200,0);
    jx_event_text(c,4,0x5aa6,1);jx_event_transition_begin(c);d[0x719c]=0;
    for(unsigned i=0;i<4;++i) d[0x719e + i]=0;
    d[0x71f7]=0;d[0x71f9]=0;d[0x7213]=1;d[0xc23c]=1;
    jx_event_refresh(c);jx_event_transition_end(c);jx_set_word(c,0x889a,8);
    if(jx_signed_word(jx_word(c,0x050c))>0) {
        uint16_t resource=jx_far_word(c,jx_word(c,0x23ae),0x520);
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((d[0x932c]+8)&255),2,resource);
    }
}

void jx_event_159_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    static const uint16_t dialogue[3]={0x5ad6,0x5ade,0x5ae6};
    for(unsigned i=0;i<3;++i) {
        jx_event_text(c,4,dialogue[i],1);
        uint16_t first=(uint16_t)(0x1160+i*33);
        d[0xc8ad+i]=(uint8_t)JX_CALL(c,0x1f2a,0x7e0e,80,80,first,c->data_segment,
                                   (uint16_t)(first-11),c->data_segment,(uint16_t)(first-22),c->data_segment);
    }
    int code=jx_signed_byte(d[0xc8ad])*100+jx_signed_byte(d[0xc8ae])*10+jx_signed_byte(d[0xc8af]);
    if(code==110) {jx_event_text(c,4,0x5af2,1);d[0xc8ac]=1;return;}
    for(unsigned i=0;i<4;++i) d[0x7866+i]=1;
    for(uint16_t frame=0;frame<3;++frame) {
        for(uint16_t i=0;i<4;++i) jx_set_word(c,(uint16_t)(0x889c+i*86+0x3c),frame);
        jx_event_refresh(c);
    }
    (void)JX_CALL(c,0x1f2a,0x5fd8,36,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
    jx_event_literal(c,-1,0x11ad,0);(void)JX_CALL(c,0x1f2a,0x2dbc,20);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
}

void jx_event_163_0000(JxEventContext *c,int16_t npc) {
    uint8_t *d=jx_ds(c);jx_event_text(c,npc,0x5b62,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,1,0x11b8,c->data_segment,3);
    (void)JX_CALL(c,0x1f2a,0xcd1c,2,0x11c4,c->data_segment,3);
    d[0x78df]=0;d[0x78e0]=0;jx_event_text(c,npc,0x5b66,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x11d0,c->data_segment,3);
    (void)JX_CALL(c,0x1f2a,0x2dbc,38);
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x11dc,c->data_segment,3);
    jx_event_text(c,npc,0x5b6a,1);jx_set_word(c,0x889a,12);
    /* Unlike most events, this path does not test DS:31B4. */
    if(jx_signed_word(jx_word(c,0x050c))>0) {
        uint16_t resource=jx_far_word(c,jx_word(c,0x23ae),0x528);
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((d[0x932c]+12)&255),2,resource);
    }
    d[0x78e1]=1;(void)JX_CALL(c,0x1f2a,0xcd1c,3,0x11e9,c->data_segment,3);
    jx_event_text(c,npc,0x5b6e,1);(void)JX_CALL(c,0x1f2a,0xcd1c,3,0x11f5,c->data_segment,3);
    d[0x78e1]=0;d[0x78c3]=1;jx_event_text(c,npc,0x5b72,1);
    jx_set_word(c,0x875a,4);jx_set_word(c,0x875c,0);
    for(unsigned i=0;i<10;++i) {d[0xc892+i]=5;d[0x7afa+i]=0;}
    (void)JX_CALL0(c,0x1f2a,0xc6a8);d[0x78de]=0;d[0x780d]=0;d[0x780e]=0;
    d[0x780f]=1;d[0x7810]=1;d[0x34fd]=1;jx_event_refresh(c);
}

void jx_event_090_0000(JxEventContext *c,int16_t npc) {
    (void)npc;(void)JX_CALL(c,0x1f2a,0xdb46,2);jx_event_text(c,2,0x5d32,1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,13,0,0xffff)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    jx_set_word(c,0x875c,1);
    static const uint16_t sequence[5][4]={{1,0,217,2},{2,0,218,2},{12,0,219,3},{18,0,220,2},{19,0,221,2}};
    for(unsigned i=0;i<5;++i) for(unsigned j=0;j<4;++j)
        jx_set_word(c,(uint16_t)(0x64e4 + i*8+j*2),sequence[i][j]);
    (void)JX_CALL(c,0x2f28,0x7062,0x106a,c->data_segment,3,1);
    jx_set_word(c,0x9334,63);
    jx_event_position(c,0x8760,13,58,3);jx_event_position(c,0x876c,12,58,3);jx_event_position(c,0x8778,11,58,3);
    jx_set_word(c,0x875c,1);jx_set_word(c,0x875e,9);jx_ds(c)[0xc07b]=2;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    uint8_t *d=jx_ds(c);d[0x7052]=0;d[0x6dbe]=0;d[0x6dbf]=0;
}

void jx_event_103_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);
    (void)JX_CALL(c,0x2f28,0x944c,0x10b2,c->data_segment,2);
    jx_event_text(c,0,0x5a32,1);jx_event_text(c,0,0x5a36,1);
    (void)JX_CALL(c,0x1f2a,0x5fd8,0x4c,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
    (void)JX_CALL(c,0x1f2a,0xdb46,2);d[0x71d8]=0;
    (void)JX_CALL(c,0x2f28,0x944c,0x10b9,c->data_segment,2);
    (void)JX_CALL(c,0x1f2a,0xcf6a,0,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)-1));
    (void)JX_CALL(c,0x1f2a,0xcd1c,0,0x10c3,c->data_segment,3);d[0x71d8]=1;
    (void)JX_CALL(c,0x1f2a,0xdba6,2);jx_event_text(c,0,0x5a3a,1);
    d[0xc21f]=1;d[0xc204]=6;d[0xc16c]=5;d[0xc16d]=5;d[0xc16e]=5;d[0x71d9]=1;d[0x71d6]=0;
    memcpy(d+0x89de,d+0x88dc,6);jx_event_refresh(c);
}
void jx_event_103_00fa(JxEventContext *c,int16_t npc) {
    (void)npc;jx_event_text(c,0,0x5a62,1);
    (void)JX_CALL(c,0x1f2a,0xcc24,jx_word(c,0x89de),(uint16_t)(jx_word(c,0x89e0)+1));
    (void)JX_CALL(c,0x2f28,0x944c,0x10c5,c->data_segment,2);jx_event_text(c,0,0x5a66,1);
    jx_ds(c)[0xc205]=5;jx_ds(c)[0x719a]=1;
}
void jx_event_103_015a(JxEventContext *c,int16_t npc) {
    (void)npc;jx_event_text(c,3,0x5a8a,1);
    uint8_t *d=jx_ds(c);d[0x71d9]=0;d[0x7212]=0;d[0x71f4]=0;d[0x71f5]=0;
    for(unsigned i=0;i<8;++i) d[0x71f6 + i]=1;
    jx_event_refresh(c);
}

void jx_event_131_0000(JxEventContext *c,int16_t npc) {
    (void)npc;uint8_t *d=jx_ds(c);if(d[0xc567]!=0) return;
    jx_event_music(c,11,2);
    (void)JX_CALL(c,0x2f28,0x56ec,'U',8);(void)JX_CALL(c,0x2f28,0x56ec,'U',6);
    jx_event_text(c,0,0x5c46,1);jx_event_text(c,2,0x5c4a,0);
    (void)JX_CALL(c,0x2f28,0x56ec,'D',6);(void)JX_CALL(c,0x2f28,0x56ec,'D',8);
    (void)JX_CALL(c,0x1f2a,0xcc24,jx_word(c,0x88dc),(uint16_t)(jx_word(c,0x88de)+1));
    jx_event_text(c,0,0x5c52,0);jx_event_transition_begin(c);
    jx_set_word(c,0xd05c,164);jx_set_word(c,0xd0a0,127);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);jx_event_refresh(c);jx_event_transition_end(c);
    d[0x34e1]=0;d[0xc567]=1;
}
void jx_event_131_011c(JxEventContext *c,int16_t npc) {
    (void)npc;(void)JX_CALL(c,0x1f2a,0x8f28,15,1);
    static const int16_t speakers[6]={0,1,1,0,1,1};
    for(unsigned i=0;i<6;++i) jx_event_text(c,speakers[i],(uint16_t)(0x5c5e + i*4),1);
    uint8_t *d=jx_ds(c);d[0x751e]=0;d[0x751f]=0;d[0x7520]=0;d[0x34e1]=1;
    jx_event_refresh(c);jx_event_music(c,18,2);
}
