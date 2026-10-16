#include "event_engine.h"
/* Original music table lives in a separate far data segment. */
static void place_music(JxEventContext *c) {
    uint16_t track=jx_word(c,0x889a);
    uint16_t seconds=jx_far_word(c,jx_word(c,0x23a0),(uint16_t)(0x0510+2*track));
    (void)JX_CALL(c,0x1832,0x224c,(uint8_t)(jx_ds(c)[0x932c]+track),2,seconds);
}

/* Place 0, 1F2A:DD82. */
void jx_place_enter_000(JxEventContext *c) {
    if (jx_ds(c)[(uint16_t)(0xb686+30*jx_word(c,0x9334)+29)]==0) (void)JX_CALL0(c,0x17d2,0x000c);
        jx_set_word(c,0x8a3e,0xdd0a);
        jx_set_word(c,0x8a40,0x1f2a);
        jx_set_word(c,0x6584,0x4e);
        jx_set_word(c,0x6586,0x53);
        jx_set_word(c,0x6588,0xdd20);
        jx_set_word(c,0x658a,0x1f2a);
}

/* Place 67, 1F2A:F7FC. */
void jx_place_enter_067(JxEventContext *c) {
    (void)JX_CALL0(c,0x17d2,0x0144);
        jx_set_word(c,0x893c,0xf7e2);
        jx_set_word(c,0x893e,0x1f2a);
}

/* Place 81, 1F2A:FE22. */
void jx_place_enter_081(JxEventContext *c) {
        jx_set_word(c,0x893c,0xfe0c);
        jx_set_word(c,0x893e,0x1f2a);
    if (jx_ds(c)[0x6fe1]!=0) (void)JX_CALL0(c,0x17d2,0x0156);
}

/* Place 118, 2F28:0AF2. */
void jx_place_enter_118(JxEventContext *c) {
    (void)JX_CALL0(c,0x17d2,0x0186);
        jx_set_word(c,0x89f0,0xaa0);
        jx_set_word(c,0x89f2,0x2f28);
}

/* Place 126, 2F28:0F70. */
void jx_place_enter_126(JxEventContext *c) {
    (void)JX_CALL0(c,0x17d2,0x0198);
        jx_set_word(c,0x88ea,0xf5a);
        jx_set_word(c,0x88ec,0x2f28);
        jx_set_word(c,0x88c6,0xf);
}

/* Place 173, 2F28:2196. */
void jx_place_enter_173(JxEventContext *c) {
    if (jx_ds(c)[0x7aa0]!=0) (void)JX_CALL0(c,0x17d2,0x01b6);
        jx_set_word(c,0x8a9c,0x2072);
        jx_set_word(c,0x8a9e,0x2f28);
}

/* Place 92, 2F28:009E. */
void jx_place_enter_092(JxEventContext *c) {
    if (jx_ds(c)[0xc16b]==0) {
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x59f2),jx_word(c,0x59f4),1);
        jx_ds(c)[0xc16b]=1;
    }
}

/* Place 147, 2F28:16FC. */
void jx_place_enter_147(JxEventContext *c) {
    jx_set_word(c,0x88ea,0x16e6);jx_set_word(c,0x88ec,0x2f28);
    uint16_t has_item=(uint16_t)JX_CALL(c,0x1f2a,0x8ca8,0x1f);
    jx_set_word(c,0x88c6,has_item?0x1f:0x20);
}

/* Place 32, 1F2A:EDC8. */
void jx_place_enter_032(JxEventContext *c) {
    if (jx_ds(c)[0xba62]==0) {
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5a92),jx_word(c,0x5a94),1);
    } else if (jx_ds(c)[0xba62]==1) {
        (void)JX_CALL0(c,0x1f2a,0xedaa);
    } else {
        jx_set_word(c,0x88d8,(uint16_t)(jx_word(c,0x31d4)-1));
        (void)JX_CALL(c,0x1f2a,0xca66,0);
    }
        jx_set_word(c,0x88e6,0xed6c);
        jx_set_word(c,0x88e8,0x1f2a);
        jx_set_word(c,0x8992,0xed94);
        jx_set_word(c,0x8994,0x1f2a);
}

/* Place 154, 2F28:19F6. */
void jx_place_enter_154(JxEventContext *c) {
    if (jx_ds(c)[0x7870]!=0) {
        (void)JX_CALL(c,0x1f2a,0xcf6a,10,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)+1));
        (void)JX_CALL(c,0x1f2a,0xcd1c,10,0x097c,c->data_segment,3);
        (void)JX_CALL(c,0x2f28,0x648e,10,jx_word(c,0x5b22),jx_word(c,0x5b24),1);
        (void)JX_CALL(c,0x1f2a,0xcf6a,10,0x2c,0x0c);
        jx_ds(c)[0x7870]=0;jx_ds(c)[0x78c0]=0;jx_ds(c)[0x7afa]=1;
    }
        jx_set_word(c,0x6584,0x2d);
        jx_set_word(c,0x6586,0x59);
        jx_set_word(c,0x6588,0x19d8);
        jx_set_word(c,0x658a,0x2f28);
        jx_set_word(c,0x658c,0x2e);
        jx_set_word(c,0x658e,0x59);
        jx_set_word(c,0x6590,0x19d8);
        jx_set_word(c,0x6592,0x2f28);
        jx_set_word(c,0x6594,0x2f);
        jx_set_word(c,0x6596,0x59);
        jx_set_word(c,0x6598,0x19d8);
        jx_set_word(c,0x659a,0x2f28);
        jx_set_word(c,0x659c,0x30);
        jx_set_word(c,0x659e,0x59);
        jx_set_word(c,0x65a0,0x19d8);
        jx_set_word(c,0x65a2,0x2f28);
}

/* Place 185, 2F28:2324. */
void jx_place_enter_185(JxEventContext *c) {
    if (jx_ds(c)[0x7c10]!=0) {
        (void)JX_CALL(c,0x1f2a,0xcf6a,8,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)+1));
        (void)JX_CALL(c,0x2f28,0x648e,8,jx_word(c,0x5b22),jx_word(c,0x5b24),1);
        (void)JX_CALL(c,0x1f2a,0xcd1c,8,0x09c2,c->data_segment,3);
        jx_ds(c)[0x7c10]=0;
        (void)JX_CALL0(c,0x2f28,0x5580);
        jx_ds(c)[0x7c26]=1;jx_ds(c)[0x7c27]=1;
    }
}

/* Place 161, 2F28:1D84. */
void jx_place_enter_161(JxEventContext *c) {
    if (jx_ds(c)[0x7943]!=0) {
        (void)JX_CALL(c,0x1f2a,0xcf6a,11,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)+1));
        (void)JX_CALL(c,0x1f2a,0xcd1c,11,0x09a1,c->data_segment,3);
        (void)JX_CALL(c,0x2f28,0x648e,11,jx_word(c,0x5a66),jx_word(c,0x5a68),1);
        (void)JX_CALL(c,0x1f2a,0xcf6a,11,0x51,0x75);
        jx_ds(c)[0x7943]=0;jx_ds(c)[0x350f]=1;
        jx_set_word(c,0x889a,12);
        if ((int16_t)jx_word(c,0x31b4)>0 && (int16_t)jx_word(c,0x050c)>0) place_music(c);
    }
    jx_set_word(c,0x8aea,0x1cd0);jx_set_word(c,0x8aec,0x2f28);
    jx_set_word(c,0x8c4a,0x1d32);jx_set_word(c,0x8c4c,0x2f28);
    jx_set_word(c,0x8cee,0x1cfa);jx_set_word(c,0x8cf0,0x2f28);
}

/* Place 213, 2F28:3440. */
void jx_place_enter_213(JxEventContext *c) {
    for (uint16_t npc=0;npc<19;npc++) {
        jx_ds(c)[0x7f50+npc]=1;
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x52),0x32d8);
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x54),0x2f28);
    }
    jx_ds(c)[0x7f63]=1;
    jx_set_word(c,0x8f50,0x331c);jx_set_word(c,0x8f52,0x2f28);
    jx_set_word(c,0x889a,6);
    if ((int16_t)jx_word(c,0x050c)>0) place_music(c);
}

/* Place 22, 1F2A:E7B4. */
void jx_place_enter_022(JxEventContext *c) {
        jx_set_word(c,0x88e6,0xe69a);
        jx_set_word(c,0x88e8,0x1f2a);
        jx_set_word(c,0x8992,0xe69a);
        jx_set_word(c,0x8994,0x1f2a);
        jx_set_word(c,0x8a3e,0xe69a);
        jx_set_word(c,0x8a40,0x1f2a);
        jx_set_word(c,0x8aea,0xe69a);
        jx_set_word(c,0x8aec,0x1f2a);
        jx_set_word(c,0x8b96,0xe69a);
        jx_set_word(c,0x8b98,0x1f2a);
        jx_set_word(c,0x8c42,0xe6de);
        jx_set_word(c,0x8c44,0x1f2a);
        jx_set_word(c,0x8c98,0xe6de);
        jx_set_word(c,0x8c9a,0x1f2a);
        jx_set_word(c,0x8cee,0xe6de);
        jx_set_word(c,0x8cf0,0x1f2a);
        jx_set_word(c,0x8d44,0xe6de);
        jx_set_word(c,0x8d46,0x1f2a);
        jx_set_word(c,0x8e9c,0xe712);
        jx_set_word(c,0x8e9e,0x1f2a);
        jx_set_word(c,0x8ef2,0xe73e);
        jx_set_word(c,0x8ef4,0x1f2a);
        jx_set_word(c,0x8f48,0xe76a);
        jx_set_word(c,0x8f4a,0x1f2a);
        jx_set_word(c,0x8f9e,0xe780);
        jx_set_word(c,0x8fa0,0x1f2a);
        jx_set_word(c,0x873e,0x9ffe);
        jx_set_word(c,0x8740,0x2f28);
    if (jx_word(c,0x08c8)!=1) {
        jx_set_word(c,0x889a,18);
        if ((int16_t)jx_word(c,0x31b4)>0 && (int16_t)jx_word(c,0x050c)>0) place_music(c);
    }
        jx_set_word(c,0x6584,7);
        jx_set_word(c,0x6586,8);
        jx_set_word(c,0x6588,0xe796);
        jx_set_word(c,0x658a,0x1f2a);
        jx_set_word(c,0x658c,8);
        jx_set_word(c,0x658e,8);
        jx_set_word(c,0x6590,0xe796);
        jx_set_word(c,0x6592,0x1f2a);
        jx_set_word(c,0x6594,9);
        jx_set_word(c,0x6596,8);
        jx_set_word(c,0x6598,0xe796);
        jx_set_word(c,0x659a,0x1f2a);
        jx_set_word(c,0x659c,0xa);
        jx_set_word(c,0x659e,8);
        jx_set_word(c,0x65a0,0xe796);
        jx_set_word(c,0x65a2,0x1f2a);
        jx_set_word(c,0x65a4,0x4b);
        jx_set_word(c,0x65a6,0x2d);
        jx_set_word(c,0x65a8,0xe702);
        jx_set_word(c,0x65aa,0x1f2a);
    jx_ds(c)[0x347f]=0;
}
