#include "event_engine.h"
#include <assert.h>

static uint32_t video_dispatch(void *opaque,uint16_t segment,uint16_t offset,
    const uint16_t *args,size_t count) {
    JxVideoBridge *b=opaque;JxEventContext *c=b->base.context;
    JxVideoDevice writer={b->bus.opaque,b->bus.out,b->bus.write_byte};
    JxVideoReadDevice reader={b->bus.opaque,b->bus.out,b->bus.read_byte};
    if(segment==0x1f2a && offset==0x752e) {
        assert(count==3);jx_portrait_draw(c,args[0],args[1],(int16_t)jx_signed_word(args[2]));return 0;
    }
    if(segment==0x2f28 && offset==0xa616) {
        assert(count==5);jx_draw_line(c,args[0],args[1],args[2],args[3],args[4]);return 0;
    }
    if(segment==0x2f28 && offset==0xa5ca) {
        assert(count==4);jx_draw_vertical(c,args[0],args[1],args[2],args[3]);return 0;
    }
    if(segment==0x1832) {
        switch(offset) {
        case 0x1250:assert(count==6);jx_video_capture_planes(c,&reader,args[0],args[1],args[2],args[3],(JxFarPointer){args[4],args[5]});return 0;
        case 0x13b0:assert(count==6);jx_video_blit_planes(c,&writer,args[0],args[1],args[2],args[3],(JxFarPointer){args[4],args[5]});return 0;
        case 0x11a8:assert(count==8);jx_video_copy_rectangle(&b->bus,args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);return 0;
        case 0x135e:assert(count==2);jx_video_copy_page(&b->bus,args[0],args[1]);return 0;
        case 0x14ce:assert(count==3);jx_video_plot(c,&writer,args[0],args[1],args[2]);return 0;
        case 0x1576:assert(count==8);jx_video_blit_silhouette(c,&writer,args[0],args[1],args[2],args[3],(JxFarPointer){args[4],args[5]},args[6],args[7]);return 0;
        case 0x1810:assert(count==1);jx_video_draw_page(c,args[0]);return 0;
        case 0x1836:assert(count==1);jx_video_show_page(&writer,args[0]);return 0;
        case 0x1884:assert(count==5);jx_video_fill_rectangle(c,&writer,args[0],args[1],args[2],args[3],args[4]);return 0;
        case 0x1a28:
            assert(count==7);jx_video_blit_transparent(c,&writer,args[0],args[1],args[2],args[3],(JxFarPointer){args[4],args[5]},args[6]);return 0;
        case 0x1c6c:
            assert(count==6);jx_video_blit(c,&writer,args[0],args[1],args[2],args[3],(JxFarPointer){args[4],args[5]});return 0;
        case 0x1e88:
            assert(count==6);jx_video_capture(c,&reader,args[0],args[1],args[2],args[3],(JxFarPointer){args[4],args[5]});return 0;
        default:break;
        }
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,args,count);
}
void jx_video_bridge_install(JxEventContext *c,JxVideoBridge *b,const JxVideoBus *bus) {
    assert(c && b && bus && c->call && c->call!=video_dispatch);
    assert(bus->out && bus->in_byte && bus->read_byte && bus->write_byte);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;b->bus=*bus;
    c->call=video_dispatch;c->opaque=b;
}
void jx_video_bridge_remove(JxEventContext *c,JxVideoBridge *b) {
    assert(c->opaque==b && c->call==video_dispatch);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
