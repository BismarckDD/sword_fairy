#include "event_engine.h"
#include <string.h>
static void init_message(JxEventContext *c,uint16_t message) {
    (void)JX_CALL(c,0x1000,0x0a80,message,c->data_segment);
}
static JxResourceInitFlow init_failure(JxEventContext *c,uint16_t message) {
    init_message(c,message);(void)JX_CALL(c,0x1000,0x01eb,1);return JX_RESOURCE_INIT_EXIT;
}
/* 1F2A:5B56. Work holds three words: frame segment, free pages, total pages.
 * Exit(1) boundaries become typed termination, even if a host callback returns.
 * File failures are not newly suppressed: original proceeds with their handles. */
JxResourceInitFlow jx_resources_initialize(JxEventContext *c,JxFarPointer work) {
    if((uint16_t)JX_CALL(c,0x1503,0x0006)) return init_failure(c,0x02c6);
    init_message(c,0x02e0);
    if((uint16_t)JX_CALL(c,0x1503,0x001d,work.offset,work.segment)) return init_failure(c,0x02ec);
    (void)JX_CALL(c,0x1000,0x0a80,0x0315,c->data_segment,jx_far_word(c,work.segment,work.offset));
    if((uint16_t)JX_CALL(c,0x1503,0x003a,(uint16_t)(work.offset+2),work.segment,(uint16_t)(work.offset+4),work.segment)) return init_failure(c,0x0328);
    (void)JX_CALL(c,0x1000,0x0a80,0x0339,c->data_segment,jx_far_word(c,work.segment,(uint16_t)(work.offset+2)));
    (void)JX_CALL(c,0x1000,0x0a80,0x0353,c->data_segment,jx_far_word(c,work.segment,(uint16_t)(work.offset+4)));
    if((uint16_t)JX_CALL(c,0x1503,0x005d,192,0x9332,c->data_segment)) return init_failure(c,0x036f);
    init_message(c,0x0386);
    uint16_t frame=jx_far_word(c,work.segment,work.offset);
    jx_set_word(c,0x5858,0);jx_set_word(c,0x585a,frame);
    jx_set_word(c,0x64de,0);jx_set_word(c,0x64e0,frame);jx_set_word(c,0x3544,frame);
    (void)JX_CALL(c,0x1f2a,0x0000);init_message(c,0x0398);
    uint32_t stream=JX_CALL(c,0x1000,0x0738,0x03b3,c->data_segment,0x03b0,c->data_segment);
    (void)JX_CALL(c,0x1000,0x0754,0x61de,c->data_segment,3,256,(uint16_t)stream,(uint16_t)(stream>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
    uint64_t bits=0;for(unsigned i=0;i<8;i++) bits|=(uint64_t)jx_ds(c)[0x238c+i]<<(8*i);
    double weight;memcpy(&weight,&bits,sizeof weight);float rounded=(float)weight;uint32_t packed;memcpy(&packed,&rounded,sizeof packed);
    (void)JX_CALL(c,0x2f28,0xa8d0,0x61de,c->data_segment,jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,(uint16_t)packed,(uint16_t)(packed>>16));
    static const uint16_t names[]={0x03c1,0x03cf,0x03e5,0x03f1};
    static const uint16_t modes[]={0x03be,0x03cc,0x03e2,0x03ee};
    static const uint16_t pages[]={75,79,183,62};
    for(unsigned resource=0;resource<4;resource++) {
        stream=JX_CALL(c,0x1000,0x0738,names[resource],c->data_segment,modes[resource],c->data_segment);
        (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)stream,(uint16_t)(stream>>16),16,0,0);
        for(uint16_t slot=0;slot<(resource==3?1:4);slot++)
            (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(pages[resource]+slot),jx_word(c,0x9332));
        (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,resource==3?3072:64000,(uint16_t)stream,(uint16_t)(stream>>16));
        (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
    }
    return JX_RESOURCE_INIT_RETURNED;
}
/* 1F2A:0000: load selected Chinese and ASCII font banks into EMS.
 * Preserve the extra full-block read and final fixed page16 mapping. */
static uint32_t font_resource_stream(JxEventContext *c) {
    return jx_word(c,0x2844)|((uint32_t)jx_word(c,0x2846)<<16);
}
static uint32_t font_resource_open(JxEventContext *c,uint16_t name,uint16_t mode) {
    uint32_t stream=JX_CALL(c,0x1000,0x0738,name,c->data_segment,mode,c->data_segment);
    jx_set_word(c,0x2844,(uint16_t)stream);jx_set_word(c,0x2846,(uint16_t)(stream>>16));return stream;
}
static void font_resource_read(JxEventContext *c,uint16_t count) {
    uint32_t stream=font_resource_stream(c);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,count,(uint16_t)stream,(uint16_t)(stream>>16));
}
static void font_resource_close(JxEventContext *c) {
    uint32_t stream=font_resource_stream(c);
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
}
void jx_font_resources_load(JxEventContext *c) {
    jx_set_word(c,0x004a,1);uint32_t size;
    if(jx_word(c,0x3456)) {
        size=0x3fe40;
        (void)jx_string_copy(c,(JxFarPointer){0x8742,c->data_segment},(JxFarPointer){0x004c,c->data_segment});
        (void)jx_string_copy(c,(JxFarPointer){0x931c,c->data_segment},(JxFarPointer){0x0050,c->data_segment});
    } else {
        size=0x41560;
        (void)jx_string_copy(c,(JxFarPointer){0x8742,c->data_segment},(JxFarPointer){0x0057,c->data_segment});
        (void)jx_string_copy(c,(JxFarPointer){0x931c,c->data_segment},(JxFarPointer){0x005d,c->data_segment});
    }
    if(!font_resource_open(c,0x8742,0x0063)) return;
    uint16_t full_pages=(uint16_t)(size/16384),remaining=(uint16_t)(size%16384);
    for(uint16_t logical=1;logical<full_pages+2;logical++) {
        (void)JX_CALL(c,0x1503,0x007d,0,logical,jx_word(c,0x9332));
        font_resource_read(c,16384);
    }
    (void)JX_CALL(c,0x1503,0x007d,0,16,jx_word(c,0x9332));font_resource_read(c,remaining);font_resource_close(c);
    if(!font_resource_open(c,0x931c,0x0066)) return;
    (void)JX_CALL(c,0x1503,0x007d,0,0,jx_word(c,0x9332));font_resource_read(c,4096);font_resource_close(c);
}
