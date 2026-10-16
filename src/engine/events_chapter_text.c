#include "event_engine.h"
static JxFarPointer chapter_work(JxFarPointer work,uint16_t field) {
    work.offset=(uint16_t)(work.offset+field);return work;
}
static void chapter_read(JxEventContext *c,uint32_t file,JxFarPointer byte) {
    (void)JX_CALL(c,0x1000,0x754,byte.offset,byte.segment,1,1,(uint16_t)file,(uint16_t)(file>>16));
}
static JxFarPointer chapter_buffer(JxEventContext *c) {
    return (JxFarPointer){jx_word(c,0x64de),jx_word(c,0x64e0)};
}
static void chapter_pointer(JxEventContext *c,uint16_t index,JxFarPointer pointer) {
    uint16_t field=(uint16_t)(0x59d6+4*index);
    jx_set_word(c,field,pointer.offset);jx_set_word(c,(uint16_t)(field+2),pointer.segment);
}
/* 1F2A:55F2. The archive stream is XOR51 encoded and terminated by raw4B.
 * Original record recognition uses the live CRT digit classification table,
 * skips three raw bytes after the first digit, and skips one after 'x'.
 * Work80 preserves unread bytes on short fread; missing terminator can loop
 * indefinitely, and record/payload bounds are not checked in the original. */
void jx_chapter_text_load(JxEventContext *c,JxFarPointer work) {
    uint16_t cursor=0,index=0;int reading=0,trim_spaces=0;
    JxFarPointer digit=chapter_work(work,18),filename=chapter_work(work,46);
    JxFarPointer raw=chapter_work(work,12),discard=chapter_work(work,14);
    (void)JX_CALL(c,0x1000,0x2a8c,digit.offset,digit.segment,0x297,c->data_segment,jx_word(c,0x8894));
    jx_far_set_byte(c,digit,1,0); /* Chapter filename keeps first decimal byte. */
    (void)JX_CALL(c,0x1000,0x2448,filename.offset,filename.segment,0x29a,c->data_segment);
    (void)JX_CALL(c,0x1000,0x23f4,filename.offset,filename.segment,digit.offset,digit.segment);
    (void)JX_CALL(c,0x1000,0x23f4,filename.offset,filename.segment,0x29e,c->data_segment);
    for(uint16_t slot=0;slot<4;slot++)
        if((uint16_t)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(175+slot),jx_word(c,0x9332))!=0) return;
    for(uint16_t i=0;i<20;i++) jx_far_set_byte(c,work,(uint16_t)(26+i),0);
    for(uint16_t i=0;i<512;i++) chapter_pointer(c,i,chapter_buffer(c));
    uint32_t file=JX_CALL(c,0x1f2a,0x5230,0x2a3,c->data_segment,filename.offset,filename.segment);
    if(!file) return;
    chapter_pointer(c,0,chapter_buffer(c));
    for(;;) {
        chapter_read(c,file,raw);
        uint8_t value=jx_far_byte(c,raw,0);
        if(value==0x4b) break;
        value^=0x51;jx_far_set_byte(c,raw,0,value);
        if(value==13 || value==10) {trim_spaces=1;continue;}
        if(trim_spaces && value==32) continue;
        trim_spaces=0;
        if((jx_word(c,(uint16_t)(0x15f7+value))&4) && !reading) {
            reading=1;
            for(unsigned i=0;i<3;i++) chapter_read(c,file,discard);
            continue;
        }
        if(!reading) continue;
        JxFarPointer buffer=chapter_buffer(c);
        if(value=='x') {
            JxFarPointer next=buffer;next.offset=(uint16_t)(next.offset+cursor+1);
            chapter_pointer(c,index,next);
            jx_far_set_byte(c,buffer,cursor,0);cursor++;index++;
            chapter_read(c,file,discard);reading=0;
        } else {jx_far_set_byte(c,buffer,cursor,value);cursor++;}
    }
    (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
}
