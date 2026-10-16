#include "event_engine.h"

static uint16_t dsp_word(JxEventContext *c,JxFarPointer p,uint16_t field) {
    return jx_far_word(c,p.segment,(uint16_t)(p.offset+field));
}
static uint8_t dsp_byte(JxEventContext *c,JxFarPointer p,uint16_t field) {
    return jx_far_byte(c,p,field);
}
static void dsp_command(JxEventContext *c,uint16_t command) {
    (void)JX_CALL(c,0x1f2a,0x27e4,jx_word(c,0x31b8),command);
}
static void dsp_mixer(JxEventContext *c,uint16_t field,uint16_t value) {
    (void)JX_CALL(c,0x1000,0x2b5e,(uint16_t)(jx_word(c,0x31b8)+field),value);
}
static uint16_t legacy_command(uint8_t mode) {return mode==1?0x1c:0x14;}
static uint16_t high_speed_command(uint8_t mode) {return mode==1?0x90:0x91;}
static void dsp_count(JxEventContext *c,uint16_t length) {
    uint16_t count=(uint16_t)(length-1);
    dsp_command(c,(uint16_t)(count&255));dsp_command(c,(uint16_t)(count>>8));
}
/* 1F2A:27E4. Busy polling has no original timeout. */
void jx_sound_dsp_write(JxEventContext *c,uint16_t base,uint16_t command) {
    while (((uint16_t)JX_CALL(c,0x1000,0x2b50,(uint16_t)(base+12))&0x80)!=0) {}
    (void)JX_CALL(c,0x1000,0x2b5e,(uint16_t)(base+12),command);
}
/* 1F2A:18F2. Incoming scratch explicitly supplies original uninitialized
 * version/command locals for unsupported version words. Normal versions
 * overwrite these values. Descriptor fields stay live after every service. */
void jx_sound_dsp_start(JxEventContext *c,uint16_t length,JxFarPointer descriptor,
    uint16_t mode_word,JxSoundDspScratch incoming_stack) {
    if (jx_word(c,0x8892)==1) return;
    if (length<=1) length=2;
    uint8_t version=incoming_stack.version,mode=(uint8_t)mode_word;
    uint16_t command=incoming_stack.command;
    int version_word=jx_signed_word(jx_word(c,0x31be));
    if (version_word>=0x400) version=4;
    else if (version_word>=0x300) {
        dsp_mixer(c,4,14);dsp_mixer(c,5,0);version=3;
    } else if (version_word>=0x201) version=2;
    else if (version_word==0x200) version=1;
    uint16_t format=dsp_word(c,descriptor,2);
    if (format>0 && format<4) {
        const uint16_t compressed[3]={0x74,0x76,0x16};
        command=compressed[format-1];
        if (mode==1) command|=9;
    } else if (version==4) {
        dsp_command(c,0x41);
        dsp_command(c,(uint16_t)(dsp_word(c,descriptor,6)>>8));
        dsp_command(c,(uint16_t)(dsp_word(c,descriptor,6)&255));
        uint16_t sample_mode;
        if (dsp_byte(c,descriptor,0)==8) {
            command=0xc0;sample_mode=dsp_byte(c,descriptor,1)==1?0:0x20;
        } else {
            command=0xb0;length/=2;sample_mode=dsp_byte(c,descriptor,1)==1?0x10:0x30;
        }
        if (mode==1) command|=4;
        dsp_command(c,command);dsp_command(c,sample_mode);dsp_count(c,length);return;
    } else if (version==3) {
        dsp_command(c,0xa0);
        if (dsp_byte(c,descriptor,1)==2) {
            jx_set_word(c,0x8892,1);dsp_command(c,0xa8);
            dsp_mixer(c,4,14);dsp_mixer(c,5,2);command=high_speed_command(mode);
        } else {
            int high=jx_signed_word(dsp_word(c,descriptor,8));
            if (high>0 || (high==0 && dsp_word(c,descriptor,6)>=0x59d8)) {
                jx_set_word(c,0x8892,1);command=high_speed_command(mode);
            } else command=legacy_command(mode);
        }
    } else if (version==2) {
        int high=jx_signed_word(dsp_word(c,descriptor,8));
        if (high>0 || (high==0 && dsp_word(c,descriptor,6)>0x32c8) ||
            dsp_byte(c,descriptor,1)==2) {
            jx_set_word(c,0x8892,1);command=high_speed_command(mode);
        } else command=legacy_command(mode);
    } else if (version==1) command=legacy_command(mode);
    dsp_command(c,0xd1);dsp_command(c,0x40);dsp_command(c,dsp_word(c,descriptor,4));
    if (mode==1 || jx_word(c,0x8892)==1) {
        dsp_command(c,0x48);dsp_count(c,length);dsp_command(c,command);
    } else {
        dsp_command(c,command);dsp_count(c,length);
    }
}
