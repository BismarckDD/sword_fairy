#include "legacy_vga_model.h"
#include <string.h>
/* Hardware semantics checked against DOSBox Staging's VGA memory device:
 * https://github.com/dosbox-staging/dosbox-staging/blob/main/src/hardware/video/vga_memory.cpp
 * This byte-per-plane implementation retains the original host plane ABI. */
void jx_legacy_vga_model_init(JxLegacyVgaModel *m,JxVideoModel *p){
    memset(m,0,sizeof(*m));m->planar=p;m->sequencer[2]=15;m->sequencer[4]=6;
    m->graphics[7]=15;m->graphics[8]=255;
    p->map_mask=15;p->read_map=0;p->mode=0;p->error=JX_VIDEO_MODEL_OK;p->start_high=0;
    p->sequencer_index=p->graphics_index=p->crtc_index=255;memset(p->latches,0,sizeof(p->latches));
}
static void legacy_out_byte(JxLegacyVgaModel *m,uint16_t port,uint8_t value){
    JxVideoModel *p=m->planar;if(p->error)return;
    switch(port){
    case 0x3c4:p->sequencer_index=value;return;
    case 0x3ce:p->graphics_index=value;return;
    case 0x3d4:p->crtc_index=value;return;
    case 0x3c5:
        if(p->sequencer_index>=5)break;
        m->sequencer[p->sequencer_index]=value;if(p->sequencer_index==2)p->map_mask=value&15;return;
    case 0x3cf:{
        if(p->graphics_index>=9)break;
        static const uint8_t masks[9]={15,15,15,31,3,0x7b,15,15,255};
        value&=masks[p->graphics_index];m->graphics[p->graphics_index]=value;
        if(p->graphics_index==4)p->read_map=value;if(p->graphics_index==5)p->mode=value;return;
    }
    case 0x3d5:
        if(p->crtc_index>=25)break;
        m->crtc[p->crtc_index]=value;if(p->crtc_index==12)p->start_high=value;return;
    default:p->error=JX_VIDEO_MODEL_BAD_PORT;return;
    }
    p->error=JX_VIDEO_MODEL_BAD_REGISTER;
}
void jx_legacy_vga_model_out(void *opaque,uint16_t port,uint16_t value,unsigned width){
    JxLegacyVgaModel *m=opaque;
    if(width!=1&&width!=2){m->planar->error=JX_VIDEO_MODEL_BAD_PORT;return;}
    legacy_out_byte(m,port,(uint8_t)value);if(width==2)legacy_out_byte(m,(uint16_t)(port+1),(uint8_t)(value>>8));
}
uint8_t jx_legacy_vga_model_in(void *opaque,uint16_t port){
    JxLegacyVgaModel *m=opaque;JxVideoModel *p=m->planar;if(p->error)return 0;
    if(port==0x3c5&&p->sequencer_index<5)return m->sequencer[p->sequencer_index];
    if(port==0x3cf&&p->graphics_index<9)return m->graphics[p->graphics_index];
    if(port==0x3d5&&p->crtc_index<25)return m->crtc[p->crtc_index];
    p->error=(port==0x3c5||port==0x3cf||port==0x3d5)?JX_VIDEO_MODEL_BAD_REGISTER:JX_VIDEO_MODEL_BAD_PORT;return 0;
}
static int legacy_address(JxLegacyVgaModel *m,uint32_t at){
    JxVideoModel *p=m->planar;if(p->error)return 0;
    if(at<0xa0000||at>=0xb0000){p->error=JX_VIDEO_MODEL_BAD_ADDRESS;return 0;}
    if((m->sequencer[4]&8)||!(m->sequencer[4]&4)||(m->graphics[5]&0x10)||(m->graphics[6]&8)){p->error=JX_VIDEO_MODEL_BAD_MODE;return 0;}
    return 1;
}
uint8_t jx_legacy_vga_model_read(void *opaque,uint32_t at){
    JxLegacyVgaModel *m=opaque;if(!legacy_address(m,at))return 0;JxVideoModel *p=m->planar;
    for(unsigned i=0;i<4;++i)p->latches[i]=p->planes[i][at-0xa0000];
    if(!(m->graphics[5]&8))return p->latches[p->read_map];
    uint8_t result=255;
    for(unsigned i=0;i<4;++i)if(m->graphics[7]&(1u<<i)){
        uint8_t expected=(m->graphics[2]&(1u<<i))?255:0;result&=(uint8_t)~(p->latches[i]^expected);
    }
    return result;
}
void jx_legacy_vga_model_write(void *opaque,uint32_t at,uint8_t value){
    JxLegacyVgaModel *m=opaque;if(!legacy_address(m,at))return;JxVideoModel *p=m->planar;
    unsigned mode=m->graphics[5]&3,rotate=m->graphics[3]&7,operation=(m->graphics[3]>>3)&3;
    uint8_t rotated=(uint8_t)((value>>rotate)|(value<<((8-rotate)&7)));
    for(unsigned i=0;i<4;++i)if(p->map_mask&(1u<<i)){
        uint8_t mask=m->graphics[8],data;
        if(mode==1){p->planes[i][at-0xa0000]=p->latches[i];continue;}
        if(mode==2)data=(value&(1u<<i))?255:0;
        else if(mode==3){data=(m->graphics[0]&(1u<<i))?255:0;mask&=rotated;}
        else data=(m->graphics[1]&(1u<<i))?((m->graphics[0]&(1u<<i))?255:0):rotated;
        if(operation==1)data&=p->latches[i];else if(operation==2)data|=p->latches[i];else if(operation==3)data^=p->latches[i];
        p->planes[i][at-0xa0000]=(uint8_t)((data&mask)|(p->latches[i]&~mask));
    }
}
JxVideoBus jx_legacy_vga_model_bus(JxLegacyVgaModel *m){return (JxVideoBus){m,jx_legacy_vga_model_out,jx_legacy_vga_model_in,jx_legacy_vga_model_read,jx_legacy_vga_model_write};}
uint8_t jx_legacy_vga_model_pixel(const JxLegacyVgaModel *m,uint16_t x,uint16_t y){
    if(x>=640||y>=480)return 0;
    uint32_t at=(uint32_t)y*80+(x>>3);uint8_t bit=(uint8_t)(128u>>(x&7)),color=0;
    for(unsigned i=0;i<4;++i)if(m->planar->planes[i][at]&bit)color|=(uint8_t)(1u<<i);return color;
}
