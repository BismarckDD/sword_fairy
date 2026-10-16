#include "event_engine.h"
/* 1000:25E0. Radix 2..36; decimal treats the input as signed 16-bit. */
uint32_t jx_itoa16(JxEventContext *c,uint16_t value,JxFarPointer destination,uint16_t radix) {
    uint16_t output=destination.offset;
    uint32_t magnitude=value;
    if(radix==10 && jx_signed_word(value)<0) {
        jx_far_set_byte(c,(JxFarPointer){output++,destination.segment},0,'-');
        magnitude=(uint32_t)(-jx_signed_word(value));
    }
    uint16_t first=output;
    do {
        uint8_t ch=(uint8_t)(magnitude%radix+'0');magnitude/=radix;
        if(ch>'9') ch=(uint8_t)(ch+39);
        jx_far_set_byte(c,(JxFarPointer){output++,destination.segment},0,ch);
    } while(magnitude);
    jx_far_set_byte(c,(JxFarPointer){output,destination.segment},0,0);
    do {
        output--;
        uint8_t left=jx_far_byte(c,(JxFarPointer){first,destination.segment},0);
        uint8_t right=jx_far_byte(c,(JxFarPointer){output,destination.segment},0);
        jx_far_set_byte(c,(JxFarPointer){output,destination.segment},0,left);
        jx_far_set_byte(c,(JxFarPointer){first++,destination.segment},0,right);
    } while((uint16_t)(first+1)<output);
    return destination.offset|((uint32_t)destination.segment<<16);
}
/* 1000:0F42, near flush helper, defined AX only. */
static JxFileWriteResult stream_flush(JxEventContext *c,uint16_t stream,const JxCrtServices *services) {
    uint16_t status=0;uint8_t flags=jx_ds(c)[(uint16_t)(stream+10)];
    if((flags&3)==2 && ((flags&8)||(jx_ds(c)[(uint16_t)(stream+0xf0)]&1))) {
        uint16_t count=(uint16_t)(jx_word(c,stream)-jx_word(c,(uint16_t)(stream+6)));
        if(jx_signed_word(count)>0) {
            uint16_t handle=jx_ds(c)[(uint16_t)(stream+11)];
            JxFarPointer source={jx_word(c,(uint16_t)(stream+6)),jx_word(c,(uint16_t)(stream+8))};
            uint16_t wrote;
            if(services) {
                JxFileWriteResult result=jx_file_write(c,handle,source,count,services->write_scratch,
                    services->available_stack,services->write,services->seek,services->hook,services->opaque);
                if(result.flow!=JX_WRITE_RETURNED) return result;
                wrote=result.value;
            } else wrote=(uint16_t)JX_CALL(c,0x1000,0x491a,handle,source.offset,source.segment,count);
            if(wrote!=count) {jx_ds(c)[(uint16_t)(stream+10)]|=32;status=65535;}
            else if(jx_ds(c)[(uint16_t)(stream+10)]&128) jx_ds(c)[(uint16_t)(stream+10)]&=(uint8_t)~2;
        }
    }
    uint16_t offset=jx_word(c,(uint16_t)(stream+6)),segment=jx_word(c,(uint16_t)(stream+8));
    jx_set_word(c,stream,offset);jx_set_word(c,(uint16_t)(stream+2),segment);jx_set_word(c,(uint16_t)(stream+4),0);return (JxFileWriteResult){JX_WRITE_RETURNED,status};
}
uint16_t jx_stream_flush(JxEventContext *c,uint16_t stream) {return stream_flush(c,stream,NULL).value;}
JxFileWriteResult jx_stream_flush_dos(JxEventContext *c,uint16_t stream,const JxCrtServices *services) {return stream_flush(c,stream,services);}
/* 1000:0C62. Only library-owned active buffers are freed. */
void jx_stream_free_buffer(JxEventContext *c,uint16_t stream) {
    uint8_t flags=jx_ds(c)[(uint16_t)(stream+10)];
    if((flags&0x83) && (flags&8)) {
        jx_free(c,(JxFarPointer){jx_word(c,(uint16_t)(stream+6)),jx_word(c,(uint16_t)(stream+8))});
        jx_ds(c)[(uint16_t)(stream+10)]&=(uint8_t)~8;
        jx_set_word(c,(uint16_t)(stream+6),0);jx_set_word(c,(uint16_t)(stream+8),0);
        jx_set_word(c,stream,0);jx_set_word(c,(uint16_t)(stream+2),0);jx_set_word(c,(uint16_t)(stream+4),0);
    }
}
/* 1000:0636. Temporary-file deletion follows close only after success. */
static JxFileWriteResult stream_close(JxEventContext *c,uint16_t stream,JxFarPointer workspace,const JxCloseServices *services) {
    uint16_t result=65535;uint8_t flags=jx_ds(c)[(uint16_t)(stream+10)];
    if(!(flags&64) && (flags&0x83)) {
        JxFileWriteResult flushed=stream_flush(c,stream,services?services->output:NULL);
        if(flushed.flow!=JX_WRITE_RETURNED) return flushed;
        result=flushed.value;
        uint16_t temporary=jx_word(c,(uint16_t)(stream+0xf4));
        jx_stream_free_buffer(c,stream);
        uint16_t handle=jx_ds(c)[(uint16_t)(stream+11)];
        uint16_t closed=services?jx_file_close(c,handle,services->close,services->opaque):
            (uint16_t)JX_CALL(c,0x1000,0x1f88,handle);
        if(jx_signed_word(closed)<0) result=65535;
        else if(temporary) {
            (void)jx_string_copy(c,workspace,(JxFarPointer){0x136c,c->data_segment});
            JxFarPointer number={(uint16_t)(workspace.offset+2),workspace.segment};
            if(jx_far_byte(c,workspace,0)=='\\') number.offset--;
            else (void)jx_string_append(c,workspace,(JxFarPointer){0x136e,c->data_segment});
            (void)jx_itoa16(c,temporary,number,10);
            uint16_t removed=services?jx_file_delete(c,workspace,services->remove,services->opaque):
                (uint16_t)JX_CALL(c,0x1000,0x4c80,workspace.offset,workspace.segment);
            if(removed) result=65535;
        }
    }
    jx_ds(c)[(uint16_t)(stream+10)]=0;return (JxFileWriteResult){JX_WRITE_RETURNED,result};
}

uint16_t jx_fclose(JxEventContext *c,uint16_t stream,JxFarPointer workspace) {return stream_close(c,stream,workspace,NULL).value;}
JxFileWriteResult jx_fclose_dos(JxEventContext *c,uint16_t stream,JxFarPointer workspace,const JxCloseServices *services) {return stream_close(c,stream,workspace,services);}
