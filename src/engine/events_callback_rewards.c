#include "event_engine.h"
static void reward_money(JxEventContext *c,uint16_t amount) {
    uint32_t value=(uint32_t)jx_word(c,0x8756)|((uint32_t)jx_word(c,0x8758)<<16);
    value+=amount;
    jx_set_word(c,0x8756,(uint16_t)value);jx_set_word(c,0x8758,(uint16_t)(value>>16));
}

/* 1F2A:E712. */
void jx_callback_1f2a_e712(JxEventContext *c,uint16_t npc) {
    if ((int8_t)jx_ds(c)[(uint16_t)(0xb91a+npc)]<=0) (void)JX_CALL(c,0x1f2a,0x966e,0,19);
}

/* 1F2A:E73E. */
void jx_callback_1f2a_e73e(JxEventContext *c,uint16_t npc) {
    if ((int8_t)jx_ds(c)[(uint16_t)(0xb91a+npc)]<=0) (void)JX_CALL(c,0x1f2a,0x966e,1,20);
}

/* 1F2A:F364. */
void jx_callback_1f2a_f364(JxEventContext *c) {
    if (jx_ds(c)[0xbcf8]==0) jx_ds(c)[0xbcf9]=5;
    if (jx_ds(c)[0xbcf8]==5) {
        jx_ds(c)[0x6c90]=1;jx_ds(c)[0x6c91]=1;jx_ds(c)[0x6ccc]=0;jx_ds(c)[0x6ccd]=0;
        (void)JX_CALL0(c,0x2f28,0x5580);
    }
}

/* 1F2A:FAA4. */
void jx_callback_1f2a_faa4(JxEventContext *c,uint16_t npc) {
    if ((int16_t)jx_word(c,0x875e)<=0) {
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5c0e),jx_word(c,0x5c10),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x8fe,c->data_segment,2);
    }
}

/* 2F28:00CE. */
void jx_callback_2f28_00ce(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[(uint16_t)(0xc16c+npc)]==5) {
        (void)JX_CALL(c,0x1f2a,0x8cea,0x1c,1);
        jx_ds(c)[(uint16_t)(0xc16c+npc)]=6;
    }
}

/* 2F28:025C. */
void jx_callback_2f28_025c(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0x71d6]!=0 && jx_ds(c)[0xc21f]==0) (void)JX_CALL(c,0x17d2,0x16e,npc);
}

/* 2F28:039E. */
void jx_callback_2f28_039e(JxEventContext *c) {
    if (jx_ds(c)[0xc23d]==0) {
        (void)JX_CALL(c,0x2f28,0x648e,4,jx_word(c,0x5a8e),jx_word(c,0x5a90),1);
        jx_ds(c)[0xc23d]=1;
    }
}

/* 2F28:0724. */
void jx_callback_2f28_0724(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0xc3e1]==0) {
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5bc2),jx_word(c,0x5bc4),1);
        jx_ds(c)[0xc3e1]=1;
    }
}

/* 2F28:0792. */
void jx_callback_2f28_0792(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0xc3ff]==0) {
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b26),jx_word(c,0x5b28),1);
        jx_ds(c)[0xc3ff]=1;
    }
}

/* 2F28:1CFA. */
void jx_callback_2f28_1cfa(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x983,c->data_segment,1);
    jx_ds(c)[0x7944]=0;jx_ds(c)[0xcd60]=5;
    reward_money(c,300);
    jx_set_word(c,0x8752,(uint16_t)(jx_word(c,0x8752)+10));
}

/* 2F28:1F52. */
void jx_callback_2f28_1f52(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x2f28,0x7a1c,npc);
    jx_ds(c)[0x79ce]=0;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5a62),jx_word(c,0x5a64),1);
    jx_ds(c)[0x7943]=1;
}

/* 2F28:2B2A. */
void jx_callback_2f28_2b2a(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5aa2),jx_word(c,0x5aa4),1);
    (void)JX_CALL0(c,0x2f28,0x373e);
    (void)JX_CALL0(c,0x2f28,0x7ec8);
    (void)JX_CALL0(c,0x1f2a,0xb196);
}

/* 2F28:2E34. */
void jx_callback_2f28_2e34(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[(uint16_t)(0xcd60+npc)]==5) {
        jx_ds(c)[(uint16_t)(0xcd60+npc)]=6;
        reward_money(c,50);
    }
}

/* 2F28:2FFC. */
void jx_callback_2f28_2ffc(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b1e),jx_word(c,0x5b20),1);
    (void)JX_CALL(c,0x2f28,0x7a1c,npc);
    jx_ds(c)[0x7e42]=0;jx_ds(c)[0xce8b]=1;
}

/* 2F28:3266. */
void jx_callback_2f28_3266(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x648e,1,jx_word(c,0x5b6a),jx_word(c,0x5b6c),1);
    jx_ds(c)[0x7ef6]=0;jx_ds(c)[0x7ef7]=0;jx_ds(c)[0x7e60]=0;
    jx_ds(c)[0xcf3f]=1;jx_ds(c)[0xcf40]=4;
    (void)JX_CALL0(c,0x2f28,0x5580);
}
