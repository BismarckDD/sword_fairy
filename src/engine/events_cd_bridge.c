#include "event_engine.h"
#include <assert.h>
static JxFarPointer cd_pointer(const uint16_t *a) {return (JxFarPointer){a[0],a[1]};}
static uint32_t cd_long(const uint16_t *a) {return a[0]|((uint32_t)a[1]<<16);}
static uint32_t cd_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxCdBridge *b=opaque;JxEventContext *c=b->base.context;
    if(segment==0x1832) switch(offset) {
    case 0x2180:assert(n==2);return jx_cd_current_status(c,&b->device,cd_pointer(a));
    case 0x21d8:assert(n==2);return jx_cd_current_disc(c,&b->device,cd_pointer(a));
    case 0x2212:assert(n==2);return jx_cd_current_position(c,&b->device,cd_pointer(a));
    case 0x224c:assert(n==3);return jx_cd_play_seconds(c,&b->device,(uint8_t)a[0],a[1],a[2],b->table,b->scratch);
    case 0x22ce:assert(n==0);return jx_cd_stop_current(c,&b->device);
    case 0x22f2:assert(n==0);return jx_cd_stop_if_active(c,&b->device,b->scratch);
    case 0x2324:assert(n==0);return jx_cd_resume_if_paused(c,&b->device,b->scratch);
    case 0x2790:assert(n==3);return jx_cd_send_request(&b->device,a[0],cd_pointer(a+1));
    case 0x2a1a:assert(n==6);return jx_cd_play(c,&b->device,(uint8_t)a[0],cd_long(a+1),cd_long(a+3),(uint8_t)a[5]);
    case 0x2a84:assert(n==1);return jx_cd_stop(c,&b->device,(uint8_t)a[0]);
    case 0x2ace:assert(n==1);return jx_cd_resume(c,&b->device,(uint8_t)a[0]);
    case 0x2b18:assert(n==5);return jx_cd_ioctl_input(c,&b->device,(uint8_t)a[0],cd_pointer(a+1),(uint8_t)a[3],(uint8_t)a[4]);
    case 0x2d10:assert(n==3);return jx_cd_audio_position(c,&b->device,(uint8_t)a[0],cd_pointer(a+1));
    case 0x2d2c:assert(n==3);return jx_cd_disc_info(c,&b->device,(uint8_t)a[0],cd_pointer(a+1));
    case 0x2d48:assert(n==3);return jx_cd_audio_status(c,&b->device,(uint8_t)a[0],cd_pointer(a+1));
    default:break;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_cd_bridge_install(JxEventContext *c,JxCdBridge *b,const JxCdDevice *device,JxFarPointer table,JxFarPointer scratch) {
    assert(c && b && device && device->request && c->call && c->call!=cd_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->device=*device;b->table=table;b->scratch=scratch;c->call=cd_dispatch;c->opaque=b;
}
void jx_cd_bridge_remove(JxEventContext *c,JxCdBridge *b) {
    assert(c->opaque==b && c->call==cd_dispatch);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
