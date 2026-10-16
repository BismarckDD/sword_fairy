#include "event_engine.h"
/* 1000:098B near worker: SI=stream, ES:BX=source, CX=request.
 * Explicit flow preserves a DIV0/overflow fault without executing host UB. */
JxBulkWriteResult jx_stream_write_chunk(JxEventContext *c,uint16_t stream,
    JxFarPointer source,uint16_t request) {
    uint16_t extra=(uint16_t)(stream+0xf0),remaining=request;
    uint16_t capacity=((jx_ds(c)[(uint16_t)(stream+10)]&12)||(jx_ds(c)[extra]&1))?jx_word(c,(uint16_t)(extra+2)):512;
    for(;;) {
        if((jx_ds(c)[(uint16_t)(stream+10)]&8)||(jx_ds(c)[extra]&1)) {
            uint16_t available=jx_word(c,(uint16_t)(stream+4));
            if(available) {
                uint16_t count=available>remaining?remaining:available;
                (void)jx_memory_copy(c,(JxFarPointer){jx_word(c,stream),jx_word(c,(uint16_t)(stream+2))},source,count);
                remaining=(uint16_t)(remaining-count);
                jx_set_word(c,(uint16_t)(stream+4),(uint16_t)(jx_word(c,(uint16_t)(stream+4))-count));
                source.offset=(uint16_t)(source.offset+count);
                jx_set_word(c,stream,(uint16_t)(jx_word(c,stream)+count));
                if(!remaining) break;
                continue;
            }
        }
        if(remaining>=capacity) {
            if((jx_ds(c)[(uint16_t)(stream+10)]&8)||(jx_ds(c)[extra]&1))
                if((uint16_t)JX_CALL(c,0x1000,0xf42,stream,c->data_segment)!=0) break;
            if(!capacity) return (JxBulkWriteResult){JX_BULK_DIVIDE_ERROR,0};
            uint16_t count=(uint16_t)(remaining-remaining%capacity);
            uint16_t result=(uint16_t)JX_CALL(c,0x1000,0x491a,jx_ds(c)[(uint16_t)(stream+11)],source.offset,source.segment,count);
            if(result!=65535) remaining=(uint16_t)(remaining-result);
            if(result==65535 || result!=count) {jx_ds(c)[(uint16_t)(stream+10)]|=32;break;}
            source.offset=(uint16_t)(source.offset+result);
        } else {
            uint16_t result=(uint16_t)JX_CALL(c,0x1000,0xb76,jx_far_byte(c,source,0),stream,c->data_segment);
            if(result==65535) break;
            source.offset++;remaining--;
            capacity=jx_word(c,(uint16_t)(extra+2));if(!capacity) capacity=1;
        }
        if(!remaining) break;
    }
    return (JxBulkWriteResult){JX_BULK_WRITE_RETURNED,(uint16_t)(request-remaining)};
}
/* 1000:08E8. Huge-pointer partitioning and complete item count mirror fread,
 * with the original 65535-byte exclusion and offset<=1 32768-byte chunks. */
JxBulkWriteResult jx_fwrite(JxEventContext *c,JxFarPointer source,
    uint16_t size,uint16_t count,uint16_t stream) {
    uint32_t requested=(uint32_t)size*count,remaining=requested;
    if(!requested) return (JxBulkWriteResult){JX_BULK_WRITE_RETURNED,0};
    for(;;) {
        if(remaining<65535 && (uint32_t)source.offset+remaining<=65536) {
            JxBulkWriteResult result=jx_stream_write_chunk(c,stream,source,(uint16_t)remaining);
            if(result.flow) return result;
            remaining-=result.value;break;
        }
        uint16_t chunk=source.offset<=1?32768:(uint16_t)(0-source.offset);
        JxBulkWriteResult result=jx_stream_write_chunk(c,stream,source,chunk);
        if(result.flow) return result;
        remaining-=result.value;if(result.value<chunk) break;
        uint32_t next=(uint32_t)source.offset+result.value;
        source.offset=(uint16_t)next;if(next>65535) source.segment=(uint16_t)(source.segment+0x1000);
        if(!remaining) break;
    }
    uint32_t items=remaining?(requested-remaining)/size:count;
    if(items>65535) return (JxBulkWriteResult){JX_BULK_DIVIDE_ERROR,0};
    return (JxBulkWriteResult){JX_BULK_WRITE_RETURNED,(uint16_t)items};
}
