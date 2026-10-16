#include "event_engine.h"
/* 1000:0C9A, near helper. Allocation failure tests segment only. */
static void stream_allocate(JxEventContext *c,uint16_t stream,const JxCrtServices *services) {
    uint32_t allocation=services?jx_malloc(c,512,services->allocate,services->resize,services->opaque):JX_CALL(c,0x1000,0x4aa1,512);
    uint16_t offset=(uint16_t)allocation,segment=(uint16_t)(allocation>>16);
    uint16_t extra=(uint16_t)(stream+0xf0);
    if(segment) {
        jx_ds(c)[(uint16_t)(stream+10)]|=8;
        jx_set_word(c,(uint16_t)(extra+2),512);
    } else {
        jx_ds(c)[(uint16_t)(stream+10)]|=4;
        jx_set_word(c,(uint16_t)(extra+2),1);
        segment=c->data_segment;offset=(uint16_t)(extra+1);
    }
    jx_set_word(c,(uint16_t)(stream+2),segment);
    jx_set_word(c,stream,offset);
    jx_set_word(c,(uint16_t)(stream+8),segment);
    jx_set_word(c,(uint16_t)(stream+6),offset);
    jx_set_word(c,(uint16_t)(stream+4),0);
}
static uint16_t stream_error(JxEventContext *c,uint16_t stream) {
    jx_ds(c)[(uint16_t)(stream+10)]|=32;return 65535;
}
static JxFileWriteResult stream_write(JxEventContext *c,uint16_t handle,JxFarPointer data,uint16_t count,const JxCrtServices *s) {
    if(s) return jx_file_write(c,handle,data,count,s->write_scratch,s->available_stack,s->write,s->seek,s->hook,s->opaque);
    return (JxFileWriteResult){0,(uint16_t)JX_CALL(c,0x1000,0x491a,handle,data.offset,data.segment,count)};
}
/* 1000:0B76. The character cell replaces the original caller stack word.
 * Low-level write, seek, and allocator remain explicit engine dependencies. */
static JxFileWriteResult stream_overflow(JxEventContext *c,uint16_t stream,JxFarPointer character,const JxCrtServices *services) {
    uint16_t flags_at=(uint16_t)(stream+10);
    uint8_t flags=jx_ds(c)[flags_at];
    if(!(flags&0x82) || (flags&0x40)) return (JxFileWriteResult){0,stream_error(c,stream)};
    jx_set_word(c,(uint16_t)(stream+4),0);
    if(flags&1) {
        if(!(flags&16)) return (JxFileWriteResult){0,stream_error(c,stream)};
        jx_set_word(c,stream,jx_word(c,(uint16_t)(stream+6)));
        flags&=(uint8_t)~1;
    }
    flags=(uint8_t)((flags|2)&~16);jx_ds(c)[flags_at]=flags;
    uint16_t extra=(uint16_t)(stream+0xf0),handle=jx_ds(c)[(uint16_t)(stream+11)];
    if(!(flags&8)) {
        int direct=(flags&4)!=0;
        if(!direct && !(jx_ds(c)[extra]&1)) {
            direct=(stream==0x137e || stream==0x138a || stream==0x13a2)
                && (jx_ds(c)[(uint16_t)(0x131e + handle)]&64);
            if(!direct) {
                stream_allocate(c,stream,services);
                direct=!(jx_ds(c)[flags_at]&8);
            }
        }
        if(direct) {
            JxFileWriteResult written=stream_write(c,handle,character,1,services);
            if(written.flow) return written;
            if(written.value!=1) return (JxFileWriteResult){0,stream_error(c,stream)};
            return (JxFileWriteResult){0,jx_far_byte(c,character,0)};
        }
    }
    uint16_t base=jx_word(c,(uint16_t)(stream+6));
    uint16_t count=(uint16_t)(jx_word(c,stream)-base);
    jx_set_word(c,stream,(uint16_t)(base+1));
    jx_set_word(c,(uint16_t)(stream+4),(uint16_t)(jx_word(c,(uint16_t)(extra+2))-1));
    uint16_t result=0;
    if(count) {
        JxFileWriteResult written=stream_write(c,handle,
            (JxFarPointer){jx_word(c,(uint16_t)(stream+6)),jx_word(c,(uint16_t)(stream+8))},count,services);
        if(written.flow) return written;
        result=written.value;
    } else if(jx_ds(c)[(uint16_t)(0x131e + handle)]&32) {
        if(services) (void)jx_file_seek(c,handle,0,2,services->seek,services->opaque);
        else (void)JX_CALL(c,0x1000,0x1fa8,handle,0,0,2);
    }
    /* Reload buffer and character after service calls, including failed writes. */
    JxFarPointer buffer={jx_word(c,(uint16_t)(stream+6)),jx_word(c,(uint16_t)(stream+8))};
    jx_far_set_byte(c,buffer,0,jx_far_byte(c,character,0));
    if(result!=count) return (JxFileWriteResult){0,stream_error(c,stream)};
    return (JxFileWriteResult){0,jx_far_byte(c,character,0)};
}


void jx_stream_allocate_buffer(JxEventContext *c,uint16_t stream) {stream_allocate(c,stream,NULL);}
uint16_t jx_stream_overflow(JxEventContext *c,uint16_t stream,JxFarPointer character) {
    return stream_overflow(c,stream,character,NULL).value;
}
JxFileWriteResult jx_stream_overflow_dos(JxEventContext *c,uint16_t stream,JxFarPointer character,const JxCrtServices *services) {
    return stream_overflow(c,stream,character,services);
}

void jx_stream_allocate_buffer_dos(JxEventContext *c,uint16_t stream,JxDosAllocateService allocate,JxDosResizeService resize,void *opaque) {
    JxCrtServices services={0};services.allocate=allocate;services.resize=resize;services.opaque=opaque;
    stream_allocate(c,stream,&services);
}
