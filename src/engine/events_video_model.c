#include "video_model.h"
#include <string.h>

void jx_video_model_init(JxVideoModel *m) {
    memset(m,0,sizeof(*m));
    m->sequencer_index=m->graphics_index=m->crtc_index=0xff;
}

static void out_byte(JxVideoModel *m,uint16_t port,uint8_t value) {
    if(m->error) return;
    switch(port) {
    case 0x3c4:m->sequencer_index=value;return;
    case 0x3ce:m->graphics_index=value;return;
    case 0x3d4:m->crtc_index=value;return;
    case 0x3c5:
        if(m->sequencer_index==2) {m->map_mask=value&15;return;}
        break;
    case 0x3cf:
        if(m->graphics_index==4) {m->read_map=value&3;return;}
        if(m->graphics_index==5) {
            if(value&0x1a) {m->error=JX_VIDEO_MODEL_BAD_MODE;return;}
            m->mode=value&0x7b;return;
        }
        break;
    case 0x3d5:
        if(m->crtc_index==0x0c) {m->start_high=value;return;}
        break;
    default:m->error=JX_VIDEO_MODEL_BAD_PORT;return;
    }
    m->error=JX_VIDEO_MODEL_BAD_REGISTER;
}
void jx_video_model_out(void *opaque,uint16_t port,uint16_t value,unsigned width) {
    JxVideoModel *m=opaque;
    if(m->error) return;
    if(width!=1 && width!=2) {m->error=JX_VIDEO_MODEL_BAD_PORT;return;}
    out_byte(m,port,(uint8_t)value);
    if(width==2) out_byte(m,(uint16_t)(port+1),(uint8_t)(value>>8));
}
static int address_valid(JxVideoModel *m,uint32_t address) {
    if(m->error) return 0;
    if(address<0xa0000 || address>=0xb0000) {
        m->error=JX_VIDEO_MODEL_BAD_ADDRESS;return 0;
    }
    return 1;
}
void jx_video_model_write(void *opaque,uint32_t address,uint8_t value) {
    JxVideoModel *m=opaque;
    if(!address_valid(m,address)) return;
    for(unsigned plane=0;plane<4;plane++)
        if(m->map_mask&(1u<<plane)) m->planes[plane][address-0xa0000]=(m->mode&1)?m->latches[plane]:value;
}
uint8_t jx_video_model_read(void *opaque,uint32_t address) {
    JxVideoModel *m=opaque;
    if(!address_valid(m,address)) return 0;
    for(unsigned plane=0;plane<4;plane++) m->latches[plane]=m->planes[plane][address-0xa0000];
    return m->latches[m->read_map];
}
JxVideoDevice jx_video_model_writer(JxVideoModel *m) {
    return (JxVideoDevice){m,jx_video_model_out,jx_video_model_write};
}
JxVideoReadDevice jx_video_model_reader(JxVideoModel *m) {
    return (JxVideoReadDevice){m,jx_video_model_out,jx_video_model_read};
}

/* The only IN currently required by recovered video routines. */
uint8_t jx_video_model_in(void *opaque,uint16_t port) {
    JxVideoModel *m=opaque;
    if(m->error) return 0;
    if(port!=0x3cf) {m->error=JX_VIDEO_MODEL_BAD_PORT;return 0;}
    if(m->graphics_index==5) return m->mode;
    if(m->graphics_index==4) return m->read_map;
    m->error=JX_VIDEO_MODEL_BAD_REGISTER;return 0;
}
JxVideoBus jx_video_model_bus(JxVideoModel *m) {
    return (JxVideoBus){m,jx_video_model_out,jx_video_model_in,
        jx_video_model_read,jx_video_model_write};
}
