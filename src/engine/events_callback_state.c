#include "event_engine.h"

/* These addresses are resident scene callbacks, not overlay entry points. */
void jx_callback_2f28_2fc8(JxEventContext *c) {
    (void)c; /* The original callback is empty. */
}
void jx_callback_1f2a_ec20(JxEventContext *c) {jx_ds(c)[0xba62]=1;}
void jx_callback_2f28_1b0c(JxEventContext *c) {jx_ds(c)[0x78a2]=1;}
void jx_callback_1f2a_f7e2(JxEventContext *c) {
    jx_ds(c)[0x34c0]=1;jx_ds(c)[0x7035]=0;jx_ds(c)[0x7036]=0;
}
void jx_callback_1f2a_e648(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00cc,npc);
    jx_ds(c)[0x346a]=1;
}
void jx_callback_1f2a_e76a(JxEventContext *c,uint16_t npc) {
    jx_ds(c)[(uint16_t)(0x68ee + npc)]=0;
}
void jx_callback_1f2a_ee7e(JxEventContext *c) {
    jx_ds(c)[0x6a08]=0;jx_ds(c)[0x6a09]=0;
    jx_ds(c)[0x6a0c]=1;jx_ds(c)[0x6a0d]=1;
}
void jx_callback_1f2a_e02e(JxEventContext *c) {
    if ((int16_t)jx_word(c,0x875c)<0) (void)JX_CALL0(c,0x17d2,0x0042);
}
void jx_callback_1f2a_e928(JxEventContext *c) {
    if ((int16_t)jx_word(c,0x875c)<0) (void)JX_CALL0(c,0x17d2,0x0042);
}
void jx_callback_1f2a_e966(JxEventContext *c,uint16_t npc) {
    jx_ds(c)[(uint16_t)(0x692a+npc)]=0;
    jx_ds(c)[0x692b]=1;
}
void jx_callback_2f28_1074(JxEventContext *c) {
    for (uint16_t flag=0x75f0;flag<=0x75f3;flag++) jx_ds(c)[flag]=1;
    jx_ds(c)[0x34ea]=1;
}
void jx_callback_1f2a_ebd0(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5aaa),jx_word(c,0x5aac),1);
}
void jx_callback_1f2a_f9f2(JxEventContext *c) {
    if (jx_ds(c)[0xbeba]==3) jx_ds(c)[0x6f7e]=1;
}
void jx_callback_2f28_18b6(JxEventContext *c) {
    jx_set_word(c,0x875e,3);jx_ds(c)[0x780c]=0;
    (void)JX_CALL0(c,0x1f2a,0xc6a8);
    (void)JX_CALL0(c,0x2f28,0x5580);
}
