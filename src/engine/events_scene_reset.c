#include "event_engine.h"

/* 1F2A:C7F2. Reset only fields written by the original; animation at
 * +3C and live coordinates/facing at +40..44 deliberately survive. */
void jx_npc_reset(JxEventContext *c) {
    for (uint16_t i=0;i<30;i++) {
        uint16_t actor=(uint16_t)(0x889c+86*i);
        for (uint16_t k=0;k<12;k++) jx_ds(c)[actor+k]=0;
        for (uint16_t k=0x0c;k<=0x2a;k+=2) {
            uint16_t value=0;
            if (k==0x0e || k==0x2a) value=0xffff;
            if (k==0x14 || k==0x16 || k==0x1c || k==0x1e) value=999;
            jx_set_word(c,(uint16_t)(actor+k),value);
        }
        for (uint16_t k=0x2c;k<0x3c;k+=2) jx_set_word(c,(uint16_t)(actor+k),0);
        jx_set_word(c,(uint16_t)(actor+0x3e),0);
        jx_set_word(c,(uint16_t)(actor+0x46),0);
        jx_set_word(c,(uint16_t)(actor+0x48),0);
        for (uint16_t k=0x4a;k<=0x52;k+=4) {
            jx_set_word(c,(uint16_t)(actor+k),0x86c4);
            jx_set_word(c,(uint16_t)(actor+k+2),0x2f28);
        }
    }
}

/* 1F2A:C9A0: ten scene object slots, ten bytes each. */
void jx_scene_objects_reset(JxEventContext *c) {
    for (uint16_t i=0;i<10;i++) {
        uint16_t object=(uint16_t)(0x92b8+10*i);
        jx_set_word(c,object,999);
        jx_set_word(c,(uint16_t)(object+2),999);
        jx_set_word(c,(uint16_t)(object+4),0xffff);
        jx_set_word(c,(uint16_t)(object+6),0xffff);
        jx_set_word(c,(uint16_t)(object+8),0);
    }
}
/* 1F2A:C6A8: initialize all 33 descriptors before loading any actor. */
void jx_scene_sprites_load(JxEventContext *c) {
    for (uint16_t i=0;i<33;i++) {
        uint16_t descriptor=(uint16_t)(0x31ce + 18*i);
        for (uint16_t k=0;k<18;k+=2) jx_set_word(c,(uint16_t)(descriptor+k),k<12?1:0);
    }
    jx_set_word(c,0x2b56,0);jx_set_word(c,0x2b58,0);
    for (uint16_t i=0;i<30;i++) {
        uint16_t actor=(uint16_t)(0x889c+86*i);
        (void)JX_CALL(c,0x1f2a,0xc264,i,jx_word(c,(uint16_t)(actor+14)));
        if (jx_word(c,(uint16_t)(0x31ce + 18*i+4))==5)
            jx_set_word(c,(uint16_t)(actor+0x3e),2);
    }
    for (uint16_t i=0;i<3;i++) {
        uint16_t member=jx_word(c,(uint16_t)(0x875a+2*i));
        uint16_t sprite=jx_signed_word(member)<0?0xffff:jx_word(c,(uint16_t)(0xd04e + 68*member+14));
        (void)JX_CALL(c,0x1f2a,0xc264,(uint16_t)(30+i),sprite);
    }
}
