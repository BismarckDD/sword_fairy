#include "video_model.h"
#include <string.h>

void jx_video_startup_model_init(JxVideoStartupModel *m) {
    memset(m,0,sizeof(*m));jx_video_model_init(&m->planar);
}
/* Explicit host BIOS boundary. Preserve plane bytes until original startup
 * clears them, allowing validation that every plane/address is written. */
void jx_video_startup_model_bios(void *opaque,uint16_t ax) {
    JxVideoStartupModel *m=opaque;
    if(m->planar.error)return;
    if(ax!=0x13){m->planar.error=JX_VIDEO_MODEL_BAD_MODE;return;}
    memset(m->sequencer,0,sizeof(m->sequencer));
    memset(m->graphics,0,sizeof(m->graphics));memset(m->crtc,0,sizeof(m->crtc));
    m->sequencer[1]=1;m->sequencer[2]=15;m->sequencer[4]=14;
    m->graphics[5]=0x40;m->graphics[6]=5;
    m->crtc[0x14]=0x40;m->crtc[0x17]=0xa3;
    m->planar.map_mask=15;m->planar.read_map=0;m->planar.mode=0x40;
    m->planar.start_high=0;
    m->planar.sequencer_index=m->planar.graphics_index=m->planar.crtc_index=0xff;
}
static void byte_out(JxVideoStartupModel *m,uint16_t port,uint8_t value) {
    JxVideoModel *p=&m->planar;
    if(p->error)return;
    if(port==0x3c5 && (p->sequencer_index==1 || p->sequencer_index==4)) {
        m->sequencer[p->sequencer_index]=value;return;
    }
    if(port==0x3cf && p->graphics_index==6){m->graphics[6]=value;return;}
    if(port==0x3d5 && (p->crtc_index==0x14 || p->crtc_index==0x17)) {
        m->crtc[p->crtc_index]=value;return;
    }
    jx_video_model_out(p,port,value,1);
    if(p->error)return;
    if(port==0x3c5)m->sequencer[2]=p->map_mask;
    if(port==0x3cf)m->graphics[p->graphics_index]=value;
    if(port==0x3d5)m->crtc[0x0c]=value;
}
static void startup_out(void *opaque,uint16_t port,uint16_t value,unsigned width) {
    JxVideoStartupModel *m=opaque;
    if(m->planar.error)return;
    if(width!=1 && width!=2){m->planar.error=JX_VIDEO_MODEL_BAD_PORT;return;}
    byte_out(m,port,(uint8_t)value);
    if(width==2)byte_out(m,(uint16_t)(port+1),(uint8_t)(value>>8));
}
static uint8_t startup_in(void *opaque,uint16_t port) {
    JxVideoStartupModel *m=opaque;JxVideoModel *p=&m->planar;
    if(p->error)return 0;
    if(port==0x3c5 && (p->sequencer_index==1 || p->sequencer_index==4))return m->sequencer[p->sequencer_index];
    if(port==0x3cf && p->graphics_index==6)return m->graphics[6];
    if(port==0x3d5 && (p->crtc_index==0x14 || p->crtc_index==0x17))return m->crtc[p->crtc_index];
    return jx_video_model_in(p,port);
}
static int planar_ready(JxVideoStartupModel *m) {
    if(m->planar.error)return 0;
    if(m->sequencer[4]&8){m->planar.error=JX_VIDEO_MODEL_BAD_MODE;return 0;}
    return 1;
}
static uint8_t startup_read(void *opaque,uint32_t address) {
    JxVideoStartupModel *m=opaque;
    return planar_ready(m)?jx_video_model_read(&m->planar,address):0;
}
static void startup_write(void *opaque,uint32_t address,uint8_t value) {
    JxVideoStartupModel *m=opaque;
    if(planar_ready(m))jx_video_model_write(&m->planar,address,value);
}
JxVideoBus jx_video_startup_model_bus(JxVideoStartupModel *m) {
    return (JxVideoBus){m,startup_out,startup_in,startup_read,startup_write};
}
