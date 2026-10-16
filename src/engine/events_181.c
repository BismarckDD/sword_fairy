#include "event_engine.h"
#include <string.h>

/* Pass a palette, packed color, destination and binary32 mixing factor.
 * The original loads a binary64 constant (0.3) then stores binary32. */
static void mix_palette(JxEventContext *c,uint16_t palette) {
    uint64_t bits=0;
    for(unsigned i=0;i<8;i++) bits|=(uint64_t)jx_ds(c)[0x23b0 + i]<<(8*i);
    double factor; memcpy(&factor,&bits,sizeof factor);
    float rounded=(float)factor;
    uint32_t float_bits; memcpy(&float_bits,&rounded,sizeof float_bits);
    (void)JX_CALL(c,0x2f28,0xa8d0,palette,c->data_segment,
        jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,
        (uint16_t)float_bits,(uint16_t)(float_bits>>16));
}

void jx_event_181_0000(JxEventContext *c,int16_t npc) {
    jx_event_text(c,npc,0x5b46,1);
    jx_event_position(c,0x64e4,5,0,93);
    jx_set_word(c,0x64ea,(uint16_t)npc);
    jx_event_music(c,7,2);
    jx_event_position(c,0x64e4,4,1,53);
    jx_event_position(c,0x64ec,15,1,54);
    jx_event_position(c,0x64f4,33,1,55);
    (void)JX_CALL(c,0x2f28,0x7062,0x1250,c->data_segment,7,1);
    /* fopen("image3.img", "rb"); keep the DOS far FILE pointer opaque. */
    uint32_t file=JX_CALL(c,0x1000,0x0738,0x125f,c->data_segment,0x125c,c->data_segment);
    if(file==0) return;
    uint16_t file_off=(uint16_t)file,file_seg=(uint16_t)(file>>16);
    (void)JX_CALL(c,0x1000,0x27fa,file_off,file_seg,16,0,0);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,64000,file_off,file_seg);
    (void)JX_CALL(c,0x1000,0x0754,0x2eb4,c->data_segment,3,256,file_off,file_seg);
    (void)JX_CALL(c,0x1000,0x0636,file_off,file_seg);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    (void)JX_CALL0(c,0x1f2a,0x7272);
    mix_palette(c,0x2eb4);
    (void)JX_CALL(c,0x1f2a,0x06f2,240,0,15);
    jx_event_text(c,npc,0x5b4e,1);
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    for(uint16_t page=0;page<2;page++) {
        (void)JX_CALL(c,0x1832,0x1810,page);
        (void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
    }
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    jx_ds(c)[0x78a2]=0; jx_ds(c)[0x78c1]=0; jx_ds(c)[0x78c2]=0;
    for(unsigned i=0;i<3;i++) {jx_ds(c)[0x78de + i]=1; jx_ds(c)[0x7afb + i]=1;}
    jx_set_word(c,(uint16_t)(0x889c + npc*86 + 12),263);
    jx_set_word(c,(uint16_t)(0x889c + npc*86 + 14),413);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);
    jx_event_refresh(c);
    (void)JX_CALL(c,0x150f,0x0006,0x61de,c->data_segment,0,256);
    mix_palette(c,0x61de);
    (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
    jx_event_text(c,npc,0x5b52,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,(uint16_t)npc,0x126a,c->data_segment,3);
    jx_ds(c)[(uint16_t)(0x7afa + npc)]=0;
}
