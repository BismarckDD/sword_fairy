#include "event_engine.h"
/* 1000:0CE6. Unknown or duplicate suffix stops parsing and still opens using
 * accumulated options; only an invalid initial access character rejects. */
static uint32_t stream_open(JxEventContext *c,JxFarPointer name,JxFarPointer mode,uint16_t sharing,uint16_t stream,const JxOpenServices *services) {
    uint8_t extra_flags=jx_ds(c)[0x15ce],stream_flags;
    uint16_t options;int commit_seen=0;
    uint8_t first=jx_far_byte(c,mode,0);
    if(first=='r') {options=0;stream_flags=1;}
    else if(first=='w') {options=0x301;stream_flags=2;}
    else if(first=='a') {options=0x109;stream_flags=2;}
    else return 0;
    int parsing=1;
    for(;;) {
        mode.offset++;uint8_t ch=jx_far_byte(c,mode,0);
        if(!ch || !parsing) break;
        switch(ch) {
        case '+':
            if(options&2) parsing=0;
            else {options=(uint16_t)((options|2)&~1u);stream_flags=128;}
            break;
        case 't':case 'b':
            if(options&0xc000) parsing=0;
            else options|=ch=='t'?0x4000:0x8000;
            break;
        case 'c':case 'n':
            if(commit_seen) parsing=0;
            else {commit_seen=1;extra_flags=ch=='c'?(uint8_t)(extra_flags|64):(uint8_t)(extra_flags&~64);}
            break;
        default:parsing=0;break;
        }
    }
    uint16_t handle=services?jx_file_open(c,name,options,sharing,0x1a4,services):
        (uint16_t)JX_CALL(c,0x1000,0x202a,name.offset,name.segment,options,sharing,0x1a4);
    if(jx_signed_word(handle)<0) return 0;
    jx_set_word(c,0x1370,(uint16_t)(jx_word(c,0x1370)+1));
    uint16_t extra=(uint16_t)(stream+0xf0);
    jx_ds(c)[(uint16_t)(stream+10)]=stream_flags;
    jx_set_word(c,(uint16_t)(stream+2),0);jx_set_word(c,stream,0);
    jx_set_word(c,(uint16_t)(stream+8),0);jx_set_word(c,(uint16_t)(stream+6),0);
    jx_ds(c)[(uint16_t)(stream+11)]=(uint8_t)handle;
    jx_ds(c)[extra]=extra_flags;
    jx_set_word(c,(uint16_t)(stream+4),0);jx_set_word(c,(uint16_t)(extra+4),0);
    return stream|((uint32_t)c->data_segment<<16);
}
uint32_t jx_stream_open(JxEventContext *c,JxFarPointer name,JxFarPointer mode,uint16_t sharing,uint16_t stream) {return stream_open(c,name,mode,sharing,stream,NULL);}
uint32_t jx_fopen_shared_dos(JxEventContext *c,JxFarPointer name,JxFarPointer mode,uint16_t sharing,const JxOpenServices *services) {
    uint32_t stream=jx_stream_find_free(c);
    if(!stream) return 0;
    return stream_open(c,name,mode,sharing,(uint16_t)stream,services);
}
uint32_t jx_fopen_dos(JxEventContext *c,JxFarPointer name,JxFarPointer mode,const JxOpenServices *services) {return jx_fopen_shared_dos(c,name,mode,0,services);}
/* 1000:0704 and 0738. Reserve/reset a stream before validating the mode. */
uint32_t jx_fopen_shared(JxEventContext *c,JxFarPointer name,JxFarPointer mode,uint16_t sharing) {
    uint32_t stream=jx_stream_find_free(c);
    if(!stream) return 0;
    return jx_stream_open(c,name,mode,sharing,(uint16_t)stream);
}
uint32_t jx_fopen(JxEventContext *c,JxFarPointer name,JxFarPointer mode) {
    return jx_fopen_shared(c,name,mode,0);
}
