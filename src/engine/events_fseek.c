#include "event_engine.h"
static uint32_t stream_seek_file(JxEventContext *c,uint16_t handle,uint32_t offset,
    uint16_t origin,JxDosSeekService service,void *opaque) {
    if(service) return jx_file_seek(c,handle,offset,origin,service,opaque);
    return JX_CALL(c,0x1000,0x1fa8,handle,(uint16_t)offset,(uint16_t)(offset>>16),origin);
}
/* 1000:287A. Translate the DOS cursor back through CRT read-ahead and
 * CR/LF expansion. File position queries can themselves move the cursor. */
uint32_t jx_ftell_dos(JxEventContext *c,uint16_t stream,JxDosSeekService service,void *opaque) {
    uint16_t extra=(uint16_t)(stream+0xf0),handle=jx_ds(c)[(uint16_t)(stream+11)];
    if(jx_signed_word(jx_word(c,(uint16_t)(stream+4)))<0) jx_set_word(c,(uint16_t)(stream+4),0);
    uint32_t position=stream_seek_file(c,handle,0,1,service,opaque);
    if(position&0x80000000u) return 0xffffffffu;
    if(!(jx_ds(c)[(uint16_t)(stream+10)]&8) && !(jx_ds(c)[extra]&1))
        return position-(uint32_t)jx_signed_word(jx_word(c,(uint16_t)(stream+4)));
    uint16_t consumed=(uint16_t)(jx_word(c,stream)-jx_word(c,(uint16_t)(stream+6)));
    uint8_t flags=jx_ds(c)[(uint16_t)(stream+10)];
    if(flags&3) {
        if(jx_ds(c)[(uint16_t)(0x131e + handle)]&128) {
            JxFarPointer p={jx_word(c,(uint16_t)(stream+6)),jx_word(c,(uint16_t)(stream+8))};
            while(p.offset<jx_word(c,stream)) {
                if(jx_far_byte(c,p,0)==10) consumed++;
                p.offset++;
            }
        }
    } else if(!(flags&128)) {
        jx_set_word(c,0x130c,22);return 0xffffffffu;
    }
    if(!position) return consumed;
    if(jx_ds(c)[(uint16_t)(stream+10)]&1) {
        if(!jx_word(c,(uint16_t)(stream+4))) consumed=0;
        else {
            uint16_t buffered=(uint16_t)(jx_word(c,stream)-jx_word(c,(uint16_t)(stream+6))+jx_word(c,(uint16_t)(stream+4)));
            if(jx_ds(c)[(uint16_t)(0x131e + handle)]&128) {
                uint32_t end=stream_seek_file(c,handle,0,2,service,opaque);
                if(end==position) {
                    uint16_t limit=(uint16_t)(buffered+jx_word(c,(uint16_t)(stream+6)));
                    JxFarPointer p={jx_word(c,(uint16_t)(stream+6)),jx_word(c,(uint16_t)(stream+8))};
                    while(p.offset<limit) {
                        if(jx_far_byte(c,p,0)==10) buffered++;
                        p.offset++;
                    }
                    if(jx_ds(c)[extra]&32) buffered++;
                } else {
                    (void)stream_seek_file(c,handle,position,0,service,opaque);
                    buffered=jx_word(c,(uint16_t)(extra+2));
                    if(jx_ds(c)[(uint16_t)(0x131e + handle)]&4) buffered++;
                }
            }
            position-=buffered;
        }
    }
    return position+consumed;
}
uint32_t jx_ftell(JxEventContext *c,uint16_t stream) {return jx_ftell_dos(c,stream,NULL,NULL);}
static JxFileWriteResult stream_seek(JxEventContext *c,uint16_t stream,uint32_t offset,
    uint16_t origin,const JxCrtServices *services) {
    if(!(jx_ds(c)[(uint16_t)(stream+10)]&0x83) || origin>2) {
        jx_set_word(c,0x130c,22);return (JxFileWriteResult){JX_WRITE_RETURNED,65535};
    }
    jx_ds(c)[(uint16_t)(stream+10)]&=(uint8_t)~16;
    if(origin==1) {
        offset+=jx_ftell_dos(c,stream,services?services->seek:NULL,services?services->opaque:NULL);
        origin=0;
    }
    if(services) {
        JxFileWriteResult flushed=jx_stream_flush_dos(c,stream,services);
        if(flushed.flow!=JX_WRITE_RETURNED) return flushed;
    } else (void)jx_stream_flush(c,stream);
    if(jx_ds(c)[(uint16_t)(stream+10)]&128) jx_ds(c)[(uint16_t)(stream+10)]&=(uint8_t)~3;
    uint32_t result=stream_seek_file(c,jx_ds(c)[(uint16_t)(stream+11)],offset,origin,
        services?services->seek:NULL,services?services->opaque:NULL);
    return (JxFileWriteResult){JX_WRITE_RETURNED,result==0xffffffffu?65535:0};
}
uint16_t jx_fseek(JxEventContext *c,uint16_t stream,uint32_t offset,uint16_t origin) {return stream_seek(c,stream,offset,origin,NULL).value;}
JxFileWriteResult jx_fseek_dos(JxEventContext *c,uint16_t stream,uint32_t offset,uint16_t origin,const JxCrtServices *services) {return stream_seek(c,stream,offset,origin,services);}
