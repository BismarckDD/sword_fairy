#include "event_engine.h"
/* 1000:1EA0, inclusive last stream pointer. Only flags83 make a slot busy. */
uint32_t jx_stream_find_free(JxEventContext *c) {
    for(uint16_t stream=0x1372;stream<=jx_word(c,0x1552);stream=(uint16_t)(stream+12)) {
        if(jx_ds(c)[(uint16_t)(stream+10)]&0x83) continue;
        jx_ds(c)[(uint16_t)(stream+10)]=0;
        jx_set_word(c,(uint16_t)(stream+4),0);
        jx_set_word(c,(uint16_t)(stream+8),0);jx_set_word(c,(uint16_t)(stream+6),0);
        jx_set_word(c,(uint16_t)(stream+2),0);jx_set_word(c,stream,0);
        jx_ds(c)[(uint16_t)(stream+11)]=255;
        return stream|((uint32_t)c->data_segment<<16);
    }
    return 0;
}
static uint16_t read_service(JxEventContext *c,uint16_t handle,JxFarPointer destination,uint16_t count,const JxReadServices *services) {
    if(services) return jx_file_read(c,handle,destination,count,services->lookahead,services->read,services->seek,services->info,services->hook,services->opaque);
    return (uint16_t)JX_CALL(c,0x1000,0x21f6,handle,destination.offset,destination.segment,count);
}
/* 1000:0ADA: refill stream then return its first unsigned byte. */
static uint16_t stream_refill(JxEventContext *c,uint16_t stream,const JxReadServices *services) {
    uint16_t flag_at=(uint16_t)(stream+10),extra=(uint16_t)(stream+0xf0);
    uint8_t flags=jx_ds(c)[flag_at];
    if(!(flags&0x83) || (flags&0x40)) return 65535;
    if(flags&2) {jx_ds(c)[flag_at]|=32;return 65535;}
    flags|=1;jx_ds(c)[flag_at]=flags;
    if(!(flags&12) && !(jx_ds(c)[extra]&1)) {
        if(services) jx_stream_allocate_buffer_dos(c,stream,services->allocate,services->resize,services->opaque);
        else jx_stream_allocate_buffer(c,stream);
    }
    uint16_t base=jx_word(c,(uint16_t)(stream+6));jx_set_word(c,stream,base);
    uint16_t handle=jx_ds(c)[(uint16_t)(stream+11)];
    uint16_t result=read_service(c,handle,(JxFarPointer){base,jx_word(c,(uint16_t)(stream+8))},jx_word(c,(uint16_t)(extra+2)),services);
    if(result==0 || result==65535) {
        jx_ds(c)[flag_at]|=result?32:16;jx_set_word(c,(uint16_t)(stream+4),0);return 65535;
    }
    if((jx_ds(c)[(uint16_t)(0x131e + handle)]&0x82)==0x82 && !(jx_ds(c)[flag_at]&0x82)) jx_ds(c)[extra]|=32;
    jx_set_word(c,(uint16_t)(stream+4),(uint16_t)(result-1));
    JxFarPointer pointer={jx_word(c,stream),jx_word(c,(uint16_t)(stream+2))};
    uint16_t value=jx_far_byte(c,pointer,0);jx_set_word(c,stream,(uint16_t)(pointer.offset+1));return value;
}
uint16_t jx_stream_refill(JxEventContext *c,uint16_t stream) {return stream_refill(c,stream,NULL);}
uint16_t jx_stream_refill_dos(JxEventContext *c,uint16_t stream,const JxReadServices *services) {return stream_refill(c,stream,services);}
/* 1000:07F7, near read worker. The destination segment is fixed per chunk. */
static uint16_t stream_read_chunk(JxEventContext *c,uint16_t stream,JxFarPointer destination,uint16_t request,const JxReadServices *services) {
    uint16_t extra=(uint16_t)(stream+0xf0),remaining=request;
    uint16_t capacity=((jx_ds(c)[(uint16_t)(stream+10)]&12)||(jx_ds(c)[extra]&1))?jx_word(c,(uint16_t)(extra+2)):512;
    for(;;) {
        if((jx_ds(c)[(uint16_t)(stream+10)]&12)||(jx_ds(c)[extra]&1)) {
            uint16_t buffered=jx_word(c,(uint16_t)(stream+4));
            if(buffered) {
                uint16_t count=buffered>remaining?remaining:buffered;
                (void)jx_memory_copy(c,destination,(JxFarPointer){jx_word(c,stream),jx_word(c,(uint16_t)(stream+2))},count);
                remaining=(uint16_t)(remaining-count);
                jx_set_word(c,(uint16_t)(stream+4),(uint16_t)(jx_word(c,(uint16_t)(stream+4))-count));
                destination.offset=(uint16_t)(destination.offset+count);
                jx_set_word(c,stream,(uint16_t)(jx_word(c,stream)+count));
                if(!remaining) break;
                continue;
            }
        }
        if(remaining>=capacity) {
            uint16_t count=(uint16_t)(remaining-remaining%capacity);
            uint16_t result=read_service(c,jx_ds(c)[(uint16_t)(stream+11)],destination,count,services);
            if(!result) {jx_ds(c)[(uint16_t)(stream+10)]|=16;break;}
            if(result==65535) {jx_ds(c)[(uint16_t)(stream+10)]|=32;break;}
            remaining=(uint16_t)(remaining-result);destination.offset=(uint16_t)(destination.offset+result);
        } else {
            uint16_t result=services?stream_refill(c,stream,services):(uint16_t)JX_CALL(c,0x1000,0x0ada,stream,c->data_segment);
            if(result==65535) break;
            jx_far_set_byte(c,destination,0,(uint8_t)result);destination.offset++;remaining--;
            capacity=jx_word(c,(uint16_t)(extra+2));
        }
        if(!remaining) break;
    }
    return (uint16_t)(request-remaining);
}
/* 1000:0754. Split huge destination at 64KiB boundaries and report items. */
static uint16_t fread_impl(JxEventContext *c,JxFarPointer destination,uint16_t size,uint16_t count,uint16_t stream,const JxReadServices *services) {
    uint32_t requested=(uint32_t)size*count,remaining=requested;
    if(!requested) return 0;
    for(;;) {
        if(remaining<65535 && (uint32_t)destination.offset+remaining<=65536) {
            remaining-=stream_read_chunk(c,stream,destination,(uint16_t)remaining,services);break;
        }
        uint16_t chunk=destination.offset<=1?32768:(uint16_t)(0-destination.offset);
        uint16_t got=stream_read_chunk(c,stream,destination,chunk,services);remaining-=got;
        if(got<chunk) break;
        uint32_t next=(uint32_t)destination.offset+got;
        destination.offset=(uint16_t)next;
        if(next>65535) destination.segment=(uint16_t)(destination.segment+0x1000);
        if(!remaining) break;
    }
    return remaining?(uint16_t)((requested-remaining)/size):count;
}

static uint16_t read_error(JxEventContext *c,uint16_t error) {
    (void)jx_dos_error(c,error);return 65535;
}
static uint16_t read_text_count(JxEventContext *c,uint16_t start,uint16_t end) {
    uint16_t count=(uint16_t)(end-start);
    return end<start?read_error(c,count):count;
}
/* 1000:21F6, binary/text read with end-of-block CR lookahead. */
uint16_t jx_file_read(JxEventContext *c,uint16_t handle,JxFarPointer destination,
    uint16_t request,JxFarPointer lookahead,JxDosReadService read,
    JxDosSeekService seek,JxDosDeviceInfoService info,JxWriteHook hook,void *opaque) {
    if(handle>=jx_word(c,0x131c)) return read_error(c,0x900);
    uint16_t flags_at=(uint16_t)(0x131e + handle);
    if(!request || (jx_ds(c)[flags_at]&2)) return 0;
    if(jx_word(c,0x18ea)==0xd6d6) hook(opaque,jx_word(c,0x18ec),handle);
    JxDosWriteResult result={0,0};read(opaque,handle,destination,request,&result);
    if(result.carry) return read_error(c,(uint16_t)(0x900|(result.ax&255)));
    if(!(jx_ds(c)[flags_at]&128)) return result.ax;
    jx_ds(c)[flags_at]&=(uint8_t)~4;
    uint16_t remaining=result.ax,input=destination.offset,output=destination.offset;
    if(!remaining) return 0;
    if(jx_far_byte(c,destination,0)==10) jx_ds(c)[flags_at]|=4;
    while(remaining) {
        uint8_t ch=jx_far_byte(c,(JxFarPointer){input,destination.segment},0);input++;
        if(ch==13) {
            if(remaining>1) {
                if(jx_far_byte(c,(JxFarPointer){input,destination.segment},0)==10) {remaining--;continue;}
            } else {
                if(jx_ds(c)[flags_at]&64) {
                    JxDosSeekResult device={0,0,0};info(opaque,handle,&device);
                    if(!(device.dx&32)) {
                        read(opaque,handle,lookahead,1,&result);
                        if(result.carry) return read_error(c,result.ax);
                    }
                    ch=10;
                } else {
                    jx_far_set_byte(c,lookahead,0,0);read(opaque,handle,lookahead,1,&result);
                    if(result.carry) return read_error(c,result.ax);
                    if(!result.ax) ch=13;
                    else if(request==1 && jx_far_byte(c,lookahead,0)==10) ch=10;
                    else {
                        JxDosSeekResult ignored={0,0,0};seek(opaque,handle,1,0xffffffffu,&ignored);
                        if(jx_far_byte(c,lookahead,0)==10) {remaining--;continue;}
                        ch=13;
                    }
                }
            }
        } else if(ch==26) {
            jx_ds(c)[flags_at]|=2;break;
        }
        jx_far_set_byte(c,(JxFarPointer){output,destination.segment},0,ch);output++;remaining--;
    }
    return read_text_count(c,destination.offset,output);
}

uint16_t jx_stream_read_chunk(JxEventContext *c,uint16_t stream,JxFarPointer destination,uint16_t count) {return stream_read_chunk(c,stream,destination,count,NULL);}
uint16_t jx_fread(JxEventContext *c,JxFarPointer destination,uint16_t size,uint16_t count,uint16_t stream) {return fread_impl(c,destination,size,count,stream,NULL);}
uint16_t jx_fread_dos(JxEventContext *c,JxFarPointer destination,uint16_t size,uint16_t count,uint16_t stream,const JxReadServices *services) {return fread_impl(c,destination,size,count,stream,services);}
/* 1000:2A1A. Exactly two getc operations, even when the first sets EOF/error.
 * The stream's segment argument is ignored by the original, which uses DS. */
static uint16_t getw_impl(JxEventContext *c,uint16_t stream,const JxReadServices *services) {
    uint16_t result=0;
    for(unsigned i=0;i<2;i++) {
        uint16_t remaining=(uint16_t)(jx_word(c,(uint16_t)(stream+4))-1),value;
        jx_set_word(c,(uint16_t)(stream+4),remaining);
        if(jx_signed_word(remaining)<0) {
            value=services?stream_refill(c,stream,services):(uint16_t)JX_CALL(c,0x1000,0x0ada,stream,c->data_segment);
        } else {
            JxFarPointer pointer={jx_word(c,stream),jx_word(c,(uint16_t)(stream+2))};
            jx_set_word(c,stream,(uint16_t)(pointer.offset+1));
            value=jx_far_byte(c,pointer,0);
        }
        result|=(uint16_t)((value&255)<<(8*i));
    }
    return (jx_ds(c)[(uint16_t)(stream+10)]&0x30)?0xffff:result;
}
uint16_t jx_getw(JxEventContext *c,uint16_t stream) {return getw_impl(c,stream,NULL);}
uint16_t jx_getw_dos(JxEventContext *c,uint16_t stream,const JxReadServices *services) {return getw_impl(c,stream,services);}

/* 1000:1EF6. Increment pointer before LES: descriptor aliases retain
 * the original increment-then-read behavior, including offset FFFF. */
static uint16_t getc_impl(JxEventContext *c,uint16_t stream,const JxReadServices *services) {
    uint16_t count=(uint16_t)(jx_word(c,(uint16_t)(stream+4))-1);
    jx_set_word(c,(uint16_t)(stream+4),count);
    if(jx_signed_word(count)<0)
        return services?stream_refill(c,stream,services):(uint16_t)JX_CALL(c,0x1000,0x0ada,stream,c->data_segment);
    jx_set_word(c,stream,(uint16_t)(jx_word(c,stream)+1));
    JxFarPointer p={(uint16_t)(jx_word(c,stream)-1),jx_word(c,(uint16_t)(stream+2))};
    return jx_far_byte(c,p,0);
}

uint16_t jx_getc(JxEventContext *c,uint16_t stream) {return getc_impl(c,stream,NULL);}
uint16_t jx_getc_dos(JxEventContext *c,uint16_t stream,const JxReadServices *services) {return getc_impl(c,stream,services);}
