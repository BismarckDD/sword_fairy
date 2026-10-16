#include "event_engine.h"

/* A word operand at offset FFFF crosses the segment's byte boundary.
 * Its effective starting offset still wraps in the 16-bit instruction. */
static void stream_store_word(JxEventContext *c,uint16_t offset,uint16_t value) {
    uint32_t address=((uint32_t)c->data_segment*16+offset)&0xfffff;
    c->memory[address]=(uint8_t)value;
    c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
}

/* 1000:0E22 (near). Temporary buffering for the three designated standard
 * streams. An allocation is accepted only when its segment is nonzero. */
uint16_t jx_standard_stream_prepare(JxEventContext *c,uint16_t stream) {
    uint16_t saved;
    if(stream==0x137e)saved=0x1556;
    else if(stream==0x138a)saved=0x155a;
    else if(stream==0x13a2)saved=0x155e;
    else return 0;
    uint16_t extra=(uint16_t)(stream-0x1372+0x1462);
    if((jx_ds(c)[(uint16_t)(stream+10)]&12)!=0 || (jx_ds(c)[extra]&1)!=0)return 0;
    uint16_t offset=jx_word(c,saved),segment=jx_word(c,(uint16_t)(saved+2));
    if((offset|segment)==0) {
        uint32_t allocated=JX_CALL(c,0x1000,0x4aa1,512);
        offset=(uint16_t)allocated;segment=(uint16_t)(allocated>>16);
        if(segment==0)return 0;
        jx_set_word(c,saved,offset);jx_set_word(c,(uint16_t)(saved+2),segment);
    }
    jx_set_word(c,(uint16_t)(stream+6),offset);
    jx_set_word(c,(uint16_t)(stream+8),segment);
    jx_set_word(c,stream,offset);jx_set_word(c,(uint16_t)(stream+2),segment);
    jx_set_word(c,(uint16_t)(stream+4),512);
    jx_set_word(c,(uint16_t)(extra+2),512);
    jx_ds(c)[(uint16_t)(stream+10)]|=2;
    jx_ds(c)[extra]=0x11;
    return 1;
}
/* 1000:0EA3 (near). Only a device handle with bit40 triggers the flush and
 * conditional pointer reset. The stream's count and flags remain untouched. */
void jx_standard_stream_finish(JxEventContext *c,uint16_t prepared,uint16_t stream) {
    uint16_t extra=(uint16_t)(stream-0x1372+0x1462);
    if((jx_ds(c)[extra]&0x10)==0)return;
    uint16_t handle=jx_ds(c)[(uint16_t)(stream+11)];
    if((jx_ds(c)[(uint16_t)(0x131e + handle)]&0x40)==0)return;
    (void)JX_CALL(c,0x1000,0x0f42,stream,c->data_segment);
    if(prepared!=0) {
        jx_ds(c)[extra]=0;stream_store_word(c,(uint16_t)(extra+2),0);
        stream_store_word(c,stream,0);stream_store_word(c,(uint16_t)(stream+2),0);
        stream_store_word(c,(uint16_t)(stream+6),0);stream_store_word(c,(uint16_t)(stream+8),0);
    }
}
/* 1000:0A80. Varargs use the emulated 16-bit stack representation. */
uint16_t jx_printf_words(JxEventContext *c,JxFarPointer format,JxFarPointer arguments) {
    uint16_t prepared=jx_standard_stream_prepare(c,0x137e);
    uint16_t result=(uint16_t)JX_CALL(c,0x1000,0x19b6,0x137e,c->data_segment,
        format.offset,format.segment,arguments.offset,arguments.segment);
    jx_standard_stream_finish(c,prepared,0x137e);
    return result;
}
uint16_t jx_printf_format(JxEventContext *c,JxFarPointer format,
    JxFarPointer arguments,JxFarPointer workspace) {
    uint16_t prepared=jx_standard_stream_prepare(c,0x137e);
    uint16_t result=jx_format_words(c,(JxFarPointer){0x137e,c->data_segment},format,arguments,workspace);
    jx_standard_stream_finish(c,prepared,0x137e);
    return result;
}
