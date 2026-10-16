#include "event_engine.h"

/* 1F2A:F5F6. */
void jx_callback_1f2a_f5f6(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xbd34]<5) jx_ds(c)[0xbd34]=5;
    jx_ds(c)[0xbde8]=1;jx_ds(c)[0xbde9]=1;
}

/* 1F2A:E32C. */
void jx_callback_1f2a_e32c(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0x66f9]==0) (void)JX_CALL(c,0x17d2,0x78,npc);
}

/* 1F2A:E54E. */
void jx_callback_1f2a_e54e(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0xb8c0]==5) (void)JX_CALL(c,0x17d2,0xae,npc);
}

/* 1F2A:E5B4. */
void jx_callback_1f2a_e5b4(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0x68b6]==0) (void)JX_CALL(c,0x17d2,0xc0,npc);
}

/* 1F2A:E6DE. */
void jx_callback_1f2a_e6de(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0x68fc]!=0) (void)JX_CALL(c,0x17d2,0xde,npc);
}

/* 2F28:02BE. */
void jx_callback_2f28_02be(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0x719b]!=0) (void)JX_CALL(c,0x17d2,0x17a,npc);
}

/* 2F28:1AD0. */
void jx_callback_2f28_1ad0(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[(uint16_t)(0xc8ce + npc)]==0) jx_ds(c)[0x7870]=1;
}

/* 2F28:2220. */
void jx_callback_2f28_2220(JxEventContext *c) {
    if (jx_ds(c)[0xcb44]==0) (void)JX_CALL(c,0x1f2a,0x8cea,0x23,1);
}

/* 1F2A:E192. */
void jx_callback_1f2a_e192(JxEventContext *c) {
    if (jx_ds(c)[0xb757]==0 && jx_ds(c)[0x692c]==0) (void)JX_CALL0(c,0x17d2,0x54);
}

/* 1F2A:E1BC. */
void jx_callback_1f2a_e1bc(JxEventContext *c) {
    if (jx_ds(c)[0xb756]==0 && jx_ds(c)[0x692c]==0) (void)JX_CALL0(c,0x17d2,0x5a);
}

/* 1F2A:E300. */
void jx_callback_1f2a_e300(JxEventContext *c,uint16_t npc) {
    if ((int8_t)jx_ds(c)[(uint16_t)(0xb7ee + npc)]<=0) (void)JX_CALL(c,0x17d2,0x72,npc);
}

/* 1F2A:E3D8. */
void jx_callback_1f2a_e3d8(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[(uint16_t)(0xb80c+npc)]!=4) (void)JX_CALL(c,0x17d2,0x84,npc);
}

/* 1F2A:EBEE. */
void jx_callback_1f2a_ebee(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xba28]>=5) {
        jx_ds(c)[0x69fc]=0;jx_ds(c)[0x69fd]=0;jx_ds(c)[0x6a06]=1;jx_ds(c)[0x6a07]=1;
        (void)JX_CALL0(c,0x2f28,0x5580);
    }
}

/* 1F2A:ED6C. */
void jx_callback_1f2a_ed6c(JxEventContext *c) {
    if (jx_ds(c)[0xba46]==0) {
        (void)JX_CALL(c,0x1f2a,0xdb46,0);jx_ds(c)[0x6a1c]=1;
    }
}

/* 1F2A:EF76. */
void jx_callback_1f2a_ef76(JxEventContext *c) {
    if (jx_ds(c)[0xbb18]==0) {
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x59da),jx_word(c,0x59dc),1);
        jx_ds(c)[0xbb18]=1;
    }
}

/* 1F2A:F440. */
void jx_callback_1f2a_f440(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x5fd8,0x1a,6);
    (void)JX_CALL(c,0x1f2a,0x155c,6);
}

/* 1F2A:F59A. */
void jx_callback_1f2a_f59a(JxEventContext *c) {
    jx_ds(c)[0x6d62]=0;jx_set_word(c,0x875e,8);
    (void)JX_CALL(c,0x1f2a,0xc264,32,jx_word(c,(uint16_t)(0xd04e + 68*jx_word(c,0x875e)+14)));
    (void)JX_CALL0(c,0x2f28,0x5580);
}

/* 1F2A:FF0C. */
void jx_callback_1f2a_ff0c(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5e36),jx_word(c,0x5e38),1);
    (void)JX_CALL(c,0x2f28,0x944c,0x925,c->data_segment,3);
}

/* 2F28:0290. */
void jx_callback_2f28_0290(JxEventContext *c) {
    int8_t state=(int8_t)jx_ds(c)[0xc205];
    if (state>=2 && state<=4) (void)JX_CALL0(c,0x17d2,0x174);
}

/* 2F28:060C. */
void jx_callback_2f28_060c(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[(uint16_t)(0xc2f2+npc)]==0) (void)JX_CALL(c,0x17d2,0x180,npc);
}

/* 2F28:1CD0. */
void jx_callback_2f28_1cd0(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0xc96a]==5) {jx_ds(c)[(uint16_t)(0xc964+npc)]=6;jx_ds(c)[0x7944]=1;}
}

/* 2F28:34C6. */
void jx_callback_2f28_34c6(JxEventContext *c) {
    if (jx_ds(c)[0xcfb6]==0 && jx_ds(c)[0x7f6e]!=0) (void)JX_CALL0(c,0x1512,0x000c);
}
