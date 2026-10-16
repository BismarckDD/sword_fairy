#include "event_engine.h"

/* Place 1, 1F2A:DE3E. */
void jx_place_enter_001(JxEventContext *c) {
    jx_set_word(c,0x893c,0xddea);
    jx_set_word(c,0x893e,0x1f2a);
    jx_set_word(c,0x8a3e,0xddea);
    jx_set_word(c,0x8a40,0x1f2a);
    jx_set_word(c,0x89e8,0xddea);
    jx_set_word(c,0x89ea,0x1f2a);
    jx_set_word(c,0x6584,0x2f);
    jx_set_word(c,0x6586,0xf);
    jx_set_word(c,0x6588,0xddcc);
    jx_set_word(c,0x658a,0x1f2a);
    jx_set_word(c,0x658c,0x30);
    jx_set_word(c,0x658e,0xf);
    jx_set_word(c,0x6590,0xddcc);
    jx_set_word(c,0x6592,0x1f2a);
    jx_set_word(c,0x6594,0x26);
    jx_set_word(c,0x6596,0x30);
    jx_set_word(c,0x6598,0xde00);
    jx_set_word(c,0x659a,0x1f2a);
    jx_set_word(c,0x659c,0x27);
    jx_set_word(c,0x659e,0x30);
    jx_set_word(c,0x65a0,0xde00);
    jx_set_word(c,0x65a2,0x1f2a);
    jx_set_word(c,0x65a4,0x28);
    jx_set_word(c,0x65a6,0x30);
    jx_set_word(c,0x65a8,0xde00);
    jx_set_word(c,0x65aa,0x1f2a);
    if (jx_ds(c)[0xb6a4]!=0) jx_ds(c)[0x6678]=0;
}

/* Place 5, 1F2A:E04A. */
void jx_place_enter_005(JxEventContext *c) {
    for (uint16_t npc=0;npc<25;npc++) {
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x4a),0xdfc0);
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x4a+2),0x1f2a);
    }
    jx_set_word(c,0x8ef6,0xe018);
    jx_set_word(c,0x8ef8,0x1f2a);
    jx_set_word(c,0x8f4c,0xe018);
    jx_set_word(c,0x8f4e,0x1f2a);
    jx_set_word(c,0x8fa2,0xe018);
    jx_set_word(c,0x8fa4,0x1f2a);
    jx_set_word(c,0x8ff8,0xe018);
    jx_set_word(c,0x8ffa,0x1f2a);
    jx_set_word(c,0x8ed2,0x14);
    jx_set_word(c,0x8f28,0x14);
    jx_set_word(c,0x8f7e,0x14);
    jx_set_word(c,0x8fd4,0x14);
    jx_set_word(c,0x8df0,0xe002);
    jx_set_word(c,0x8df2,0x1f2a);
    jx_set_word(c,0x8bec,0xdfd6);
    jx_set_word(c,0x8bee,0x1f2a);
    jx_set_word(c,0x8bf0,0xdfec);
    jx_set_word(c,0x8bf2,0x1f2a);
    jx_set_word(c,0x8bcc,0x13);
    jx_set_word(c,0x6584,0x4e);
    jx_set_word(c,0x6586,0x76);
    jx_set_word(c,0x6588,0xdfa2);
    jx_set_word(c,0x658a,0x1f2a);
    jx_set_word(c,0x658c,0x4f);
    jx_set_word(c,0x658e,0x76);
    jx_set_word(c,0x6590,0xdfa2);
    jx_set_word(c,0x6592,0x1f2a);
    jx_set_word(c,0x6594,0x50);
    jx_set_word(c,0x6596,0x76);
    jx_set_word(c,0x6598,0xdfa2);
    jx_set_word(c,0x659a,0x1f2a);
    jx_set_word(c,0x659c,0x51);
    jx_set_word(c,0x659e,0x76);
    jx_set_word(c,0x65a0,0xdfa2);
    jx_set_word(c,0x65a2,0x1f2a);
    jx_set_word(c,0x65a4,0x7e);
    jx_set_word(c,0x65a6,0x5c);
    jx_set_word(c,0x65a8,0xe02e);
    jx_set_word(c,0x65aa,0x1f2a);
}

/* Place 20, 1F2A:E5F6. */
void jx_place_enter_020(JxEventContext *c) {
    if (jx_ds(c)[0xb8df]==0) {
        jx_set_word(c,0x6584,0x3b);
        jx_set_word(c,0x6586,0x76);
        jx_set_word(c,0x6588,0xe5d8);
        jx_set_word(c,0x658a,0x1f2a);
    }
    jx_set_word(c,0x899a,0xe588);
    jx_set_word(c,0x899c,0x1f2a);
    jx_set_word(c,0x8a3e,0xe59e);
    jx_set_word(c,0x8a40,0x1f2a);
    jx_set_word(c,0x8a94,0xe5b4);
    jx_set_word(c,0x8a96,0x1f2a);
}

/* Place 43, 1F2A:F0B0. */
void jx_place_enter_043(JxEventContext *c) {
    if (jx_ds(c)[0x6804]==0) jx_ds(c)[0x6b65]=1;
}

/* Place 121, 2F28:0BF8. */
void jx_place_enter_121(JxEventContext *c) {
    for (uint16_t trigger=0;trigger<9;trigger++) {
        uint16_t p=(uint16_t)(0x6584+8*trigger);
        jx_set_word(c,p+2,0x16);
        jx_set_word(c,p+4,0x0b22);
        jx_set_word(c,p+6,0x2f28);
    }
    jx_set_word(c,0x6584,0x13);
    jx_set_word(c,0x658c,0x14);
    jx_set_word(c,0x6594,0x21);
    jx_set_word(c,0x659c,0x22);
    jx_set_word(c,0x65a4,0x23);
    jx_set_word(c,0x65ac,0x24);
    jx_set_word(c,0x65b4,0x25);
    jx_set_word(c,0x65bc,0x44);
    jx_set_word(c,0x65c4,0x45);
}

/* Place 171, 2F28:200E. */
void jx_place_enter_171(JxEventContext *c) {
    for (uint16_t npc=0;npc<8;npc++) {
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x52),0x1fc4);
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x52+2),0x2f28);
    }
}

/* Place 172, 2F28:2040. */
void jx_place_enter_172(JxEventContext *c) {
    for (uint16_t npc=0;npc<8;npc++) {
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x52),0x1fc4);
        jx_set_word(c,(uint16_t)(0x889c+86*npc+0x52+2),0x2f28);
    }
}
