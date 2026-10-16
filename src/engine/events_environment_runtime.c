#include "event_engine.h"
static uint32_t environment_pack(uint16_t offset,uint16_t segment) {return offset|((uint32_t)segment<<16);}
/* 1000:24C8, DF=0. Fixed segment offsets wrap; zero pads all remaining
 * bytes. Source/destination overlap is deliberately not repaired. */
uint32_t jx_string_copy_bounded(JxEventContext *c,JxFarPointer dest,JxFarPointer source,uint16_t count) {
    uint32_t result=environment_pack(dest.offset,dest.segment);
    while(count!=0) {
        uint8_t ch=jx_far_byte(c,source,0);source.offset++;
        if(ch==0) {
            do {jx_far_set_byte(c,dest,0,0);dest.offset++;}while(--count!=0);
            break;
        }
        jx_far_set_byte(c,dest,0,ch);dest.offset++;count--;
    }
    return result;
}
/* 1000:24F2. Unsigned byte ordering, bounded by the first string's NUL. */
int16_t jx_string_compare_bounded(JxEventContext *c,JxFarPointer left,JxFarPointer right,uint16_t count) {
    for(unsigned i=0;i<count;i++) {
        uint8_t a=jx_far_byte(c,left,(uint16_t)i),b=jx_far_byte(c,right,(uint16_t)i);
        if(a!=b)return a<b?-1:1;
        if(a==0)return 0;
    }
    return 0;
}
/* 1000:252E ->258A. Only space/tab are skipped; arithmetic wraps32 bits. */
uint32_t jx_atol32(JxEventContext *c,JxFarPointer text) {
    uint8_t ch;do {ch=jx_far_byte(c,text,0);text.offset++;}while(ch==' ' || ch=='\t');
    unsigned negative=(ch=='-');
    if(ch=='-' || ch=='+') {ch=jx_far_byte(c,text,0);text.offset++;}
    uint32_t value=0;
    while(ch>='0' && ch<='9') {
        value=value*10+(ch-'0');ch=jx_far_byte(c,text,0);text.offset++;
    }
    return negative?0u-value:value;
}
static JxFarPointer environment_slot(JxEventContext *c,JxFarPointer table) {
    JxFarPointer p={jx_far_word(c,table.segment,table.offset),
        jx_far_word(c,table.segment,(uint16_t)(table.offset+2))};return p;
}
/* LES reads its complete pointer physically, unlike the separate word
 * effective-address reads above when table offsets wrap. */
static JxFarPointer environment_les(JxEventContext *c,JxFarPointer table) {
    uint32_t at=(uint32_t)table.segment*16+table.offset;
    JxFarPointer p={(uint16_t)(c->memory[at&0xfffff]|(uint16_t)c->memory[(at+1)&0xfffff]<<8),
        (uint16_t)(c->memory[(at+2)&0xfffff]|(uint16_t)c->memory[(at+3)&0xfffff]<<8)};return p;
}
/* 1000:26A4. Environment is a Far-pointer table at1338, cached once.
 * Length/compare callbacks can change table entries; re-read at each site. */
uint32_t jx_environment_get(JxEventContext *c,JxFarPointer key) {
    JxFarPointer table={jx_word(c,0x1338),jx_word(c,0x133a)};
    if((table.offset|table.segment)==0 || (key.offset|key.segment)==0)return 0;
    uint16_t length=(uint16_t)JX_CALL(c,0x1000,0x24ae,key.offset,key.segment);
    for(;;) {
        JxFarPointer p=environment_slot(c,table);
        if((p.offset|p.segment)==0)return 0;
        uint16_t size=(uint16_t)JX_CALL(c,0x1000,0x24ae,p.offset,p.segment);
        if(size>length && jx_far_byte(c,environment_les(c,table),length)=='=') {
            p=environment_slot(c,table);
            uint16_t equal=(uint16_t)JX_CALL(c,0x1000,0x24f2,p.offset,p.segment,key.offset,key.segment,length);
            if(equal==0) {
                p=environment_slot(c,table);
                return environment_pack((uint16_t)(p.offset+length+1),p.segment);
            }
        }
        table.offset=(uint16_t)(table.offset+4);
    }
}
