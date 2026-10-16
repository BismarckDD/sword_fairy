#include "event_engine.h"

static JxFarPointer voc_work(JxFarPointer work,uint16_t field) {
    return (JxFarPointer){(uint16_t)(work.offset+field),work.segment};
}
static JxFarPointer voc_source(JxEventContext *c,uint16_t cursor) {
    uint32_t offset=(uint32_t)jx_word(c,0x2eb0)+cursor;
    return (JxFarPointer){(uint16_t)offset,
        (uint16_t)(jx_word(c,0x2eb2)+((offset>>16)<<12))};
}
static void voc_read(JxEventContext *c,uint32_t file,JxFarPointer target,uint16_t size) {
    (void)JX_CALL(c,0x1000,0x754,target.offset,target.segment,size,1,(uint16_t)file,(uint16_t)(file>>16));
}
static uint16_t voc_compare(JxEventContext *c,JxFarPointer text,uint16_t signature,uint16_t size) {
    return (uint16_t)JX_CALL(c,0x1000,0x30ea,text.offset,text.segment,signature,c->data_segment,size);
}
static void voc_copy(JxEventContext *c,uint16_t destination,uint16_t cursor,uint16_t size) {
    JxFarPointer source=voc_source(c,cursor);
    (void)JX_CALL(c,0x1000,0x3146,destination,c->data_segment,source.offset,source.segment,size);
}
static uint8_t voc_byte(JxEventContext *c,uint16_t cursor) {
    return jx_far_byte(c,voc_source(c,cursor),0);
}
static uint32_t voc_length(JxEventContext *c,uint16_t entry) {
    return (uint32_t)jx_word(c,entry)|((uint32_t)jx_word(c,(uint16_t)(entry+2))<<16);
}
static void voc_set_length(JxEventContext *c,uint16_t entry,uint32_t value) {
    jx_set_word(c,entry,(uint16_t)value);jx_set_word(c,(uint16_t)(entry+2),(uint16_t)(value>>16));
}
static uint32_t voc_divide(JxEventContext *c,uint32_t numerator,uint32_t denominator) {
    return JX_CALL(c,0x1000,0x3384,(uint16_t)numerator,(uint16_t)(numerator>>16),
        (uint16_t)denominator,(uint16_t)(denominator>>16));
}
static void voc_rate(JxEventContext *c,uint16_t descriptor,uint32_t rate) {
    jx_set_word(c,(uint16_t)(descriptor+6),(uint16_t)rate);
    jx_set_word(c,(uint16_t)(descriptor+8),(uint16_t)(rate>>16));
}
/* 1F2A:1D6E. work is 46 private bytes, retaining the original initial
 * bytes for short header reads and checksum-failure status. Only the first
 * sound block is parsed. The original leaks FILE on invalid headers and
 * reads one final EOF byte into its temporary sound buffer. */
uint8_t jx_voc_load(JxEventContext *c,JxFarPointer filename,uint16_t channel,JxFarPointer work) {
    uint32_t file=JX_CALL(c,0x1000,0x738,filename.offset,filename.segment,0x96,c->data_segment);
    if (!file) return 0xff;
    JxFarPointer header=voc_work(work,24);
    voc_read(c,file,header,20);
    if (voc_compare(c,header,0x99,19)!=0 || jx_far_byte(c,header,19)!=0x1a) {
        if (voc_compare(c,header,0xad,4)==0 && voc_compare(c,voc_work(work,32),0xb2,8)==0)
            jx_far_set_byte(c,work,6,3);
        else jx_far_set_byte(c,work,6,1);
        return jx_far_byte(c,work,6);
    }
    voc_read(c,file,voc_work(work,16),2);
    voc_read(c,file,voc_work(work,10),2);
    voc_read(c,file,voc_work(work,8),2);
    uint16_t version=jx_far_word(c,work.segment,(uint16_t)(work.offset+10));
    uint16_t checksum=jx_far_word(c,work.segment,(uint16_t)(work.offset+8));
    if ((uint16_t)((uint16_t)~version+0x1234)!=checksum) return jx_far_byte(c,work,6);
    jx_far_set_byte(c,work,6,2);
    uint16_t start=jx_far_word(c,work.segment,(uint16_t)(work.offset+16));
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),start,0,0);
    JxFarPointer buffer={jx_word(c,0x2eb0),jx_word(c,0x2eb2)};
    JxFarPointer stream={(uint16_t)file,(uint16_t)(file>>16)};
    while ((jx_far_byte(c,stream,10)&0x10)==0) {
        uint8_t value=(uint8_t)JX_CALL(c,0x1000,0x1ef6,stream.offset,stream.segment);
        jx_far_set_byte(c,buffer,0,value);
        uint32_t next=(uint32_t)buffer.offset+1;
        buffer.offset=(uint16_t)next;buffer.segment=(uint16_t)(buffer.segment+((next>>16)<<12));
    }
    (void)JX_CALL(c,0x1000,0x636,stream.offset,stream.segment);
    uint16_t page_entry=(uint16_t)(0x28f0+4u*channel);
    uint16_t length_entry=(uint16_t)(0x28c0+4u*channel);
    uint16_t descriptor=(uint16_t)(0x2848+10u*channel);
    for (uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(151+slot+jx_word(c,page_entry)),jx_word(c,0x9332));
    uint16_t cursor=0;
    uint8_t type=voc_byte(c,cursor++);
    jx_ds(c)[(uint16_t)(0x2920+channel)]=type;
    switch (type) {
    case 1: {
        voc_set_length(c,length_entry,0);voc_copy(c,length_entry,cursor,3);cursor+=3;
        jx_set_word(c,(uint16_t)(descriptor+4),voc_byte(c,cursor++));
        jx_set_word(c,(uint16_t)(descriptor+2),(uint16_t)jx_signed_byte(voc_byte(c,cursor++)));
        voc_set_length(c,length_entry,voc_length(c,length_entry)-2);
        jx_ds(c)[(uint16_t)(descriptor+1)]=1;jx_ds(c)[descriptor]=8;
        uint32_t denominator=(uint32_t)((int32_t)jx_word(c,(uint16_t)(descriptor+4))-256);
        uint32_t rate=voc_divide(c,0xfff0bdc0u,denominator);voc_rate(c,descriptor,rate);break;
    }
    case 8: {
        cursor+=3;voc_copy(c,(uint16_t)(descriptor+4),cursor,2);cursor+=2;
        jx_set_word(c,(uint16_t)(descriptor+2),(uint16_t)jx_signed_byte(voc_byte(c,cursor++)));
        jx_ds(c)[(uint16_t)(descriptor+1)]=voc_byte(c,cursor++);
        jx_ds(c)[(uint16_t)(descriptor+1)]++;
        jx_set_word(c,(uint16_t)(descriptor+4),(uint16_t)(jx_word(c,(uint16_t)(descriptor+4))>>8));
        cursor++;voc_set_length(c,length_entry,0);voc_copy(c,length_entry,cursor,3);cursor+=5;
        voc_set_length(c,length_entry,voc_length(c,length_entry)-2);jx_ds(c)[descriptor]=8;
        uint32_t channels=jx_ds(c)[(uint16_t)(descriptor+1)];
        uint32_t denominator=(uint32_t)((int32_t)jx_word(c,(uint16_t)(descriptor+4))-256);
        uint32_t rate=voc_divide(c,0xfff0bdc0u,denominator);
        rate=voc_divide(c,rate,channels);voc_rate(c,descriptor,rate);break;
    }
    case 9: {
        voc_set_length(c,length_entry,0);voc_copy(c,length_entry,cursor,3);cursor+=3;
        voc_copy(c,(uint16_t)(descriptor+6),cursor,4);cursor+=4;
        jx_ds(c)[descriptor]=voc_byte(c,cursor++);
        voc_copy(c,(uint16_t)(descriptor+1),cursor,1);cursor++;
        voc_copy(c,(uint16_t)(descriptor+2),cursor,2);cursor+=6;
        voc_set_length(c,length_entry,voc_length(c,length_entry)-12);
        uint32_t rate=(uint32_t)jx_word(c,(uint16_t)(descriptor+6))|
            ((uint32_t)jx_word(c,(uint16_t)(descriptor+8))<<16);
        uint32_t channels=jx_ds(c)[(uint16_t)(descriptor+1)];
        uint32_t product=JX_CALL(c,0x1000,0x341e,(uint16_t)channels,0,(uint16_t)rate,(uint16_t)(rate>>16));
        uint32_t constant=voc_divide(c,0xfff0bec0u,product);
        jx_set_word(c,(uint16_t)(descriptor+4),(uint16_t)(constant&255));break;
    }
    default:break;
    }
    jx_set_word(c,(uint16_t)(page_entry+2),(uint16_t)voc_length(c,length_entry));
    if (jx_word(c,(uint16_t)(page_entry+2))>0xfff0) jx_set_word(c,(uint16_t)(page_entry+2),0xfff0);
    for (uint16_t next_channel=channel;jx_signed_word(next_channel)<10;next_channel++) {
        uint16_t first=(uint16_t)(jx_word(c,page_entry)+jx_word(c,(uint16_t)(page_entry+2))/16384+1);
        jx_set_word(c,(uint16_t)(0x28f0+4u*(uint16_t)(next_channel+1)),first);
    }
    JxFarPointer source=voc_source(c,cursor);
    (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x5858),jx_word(c,0x585a),
        source.offset,source.segment,jx_word(c,(uint16_t)(page_entry+2)));
    return jx_far_byte(c,work,6);
}
