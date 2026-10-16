#include "event_engine.h"

/* 1000:0FCA near. Mode 1 counts successful flushes of flags83 streams;
 * mode 0 returns an accumulated error for writable streams. */
uint16_t jx_stream_flush_all_worker(JxEventContext *c,uint16_t mode) {
    uint16_t stream=0x1372,count=0,error=0;
    while(stream<=jx_word(c,0x1552)) {
        uint8_t flags=jx_ds(c)[(uint16_t)(stream+10)];
        if(mode==1 && (flags&0x83)!=0) {
            uint16_t result=(uint16_t)JX_CALL(c,0x1000,0x0eea,stream,c->data_segment);
            if(result!=0xffff)count=(uint16_t)(count+1);
        } else if(mode==0 && (flags&2)!=0) {
            if((uint16_t)JX_CALL(c,0x1000,0x0eea,stream,c->data_segment)==0xffff)error=0xffff;
        }
        stream=(uint16_t)(stream+12);
    }
    return mode==1?count:error;
}
uint16_t jx_stream_flush_all(JxEventContext *c) {
    return jx_stream_flush_all_worker(c,1);
}
/* 1000:0EEA. Its Far NULL test and ES-based handle load are intentional. */
uint16_t jx_fflush(JxEventContext *c,JxFarPointer stream) {
    if((stream.offset|stream.segment)==0)return jx_stream_flush_all_worker(c,0);
    if((uint16_t)JX_CALL(c,0x1000,0x0f42,stream.offset,stream.segment)!=0)return 0xffff;
    uint16_t extra=(uint16_t)(stream.offset-0x1372+0x1462);
    if((jx_ds(c)[extra]&0x40)==0)return 0;
    uint16_t handle=jx_far_byte(c,stream,11);
    return (uint16_t)JX_CALL(c,0x1000,0x2af6,handle)!=0?0xffff:0;
}
/* 1000:27D4. Close even inactive slots beyond the five standard streams. */
uint16_t jx_stream_close_extra(JxEventContext *c) {
    uint16_t stream=0x13ae,count=0;
    while(stream<=jx_word(c,0x1552)) {
        uint16_t result=(uint16_t)JX_CALL(c,0x1000,0x0636,stream,c->data_segment);
        if(result!=0xffff)count=(uint16_t)(count+1);
        stream=(uint16_t)(stream+12);
    }
    return count;
}
/* 1000:0AC4, registered in the original CRT exit table. */
void jx_crt_stdio_cleanup(JxEventContext *c) {
    (void)jx_stream_flush_all(c);
    if(jx_ds(c)[0x1349]!=0)(void)jx_stream_close_extra(c);
}
