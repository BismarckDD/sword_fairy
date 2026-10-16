#include "event_engine.h"
static uint8_t animation_byte(JxEventContext *c,JxFarPointer *source) {
    uint8_t value=jx_far_byte(c,*source,0);source->offset=(uint16_t)(source->offset+1);return value;
}
static uint16_t animation_word(JxEventContext *c,JxFarPointer *source) {
    uint16_t value=jx_far_word(c,source->segment,source->offset);source->offset=(uint16_t)(source->offset+2);return value;
}
static void animation_fill(JxEventContext *c,JxFarPointer *destination,uint16_t count,uint8_t value) {
    for(uint16_t i=0;i<count;i++) {
        jx_far_set_byte(c,*destination,0,value);destination->offset=(uint16_t)(destination->offset+1);
    }
}
static void animation_copy(JxEventContext *c,JxFarPointer *destination,JxFarPointer *source,uint16_t count) {
    for(uint16_t i=0;i<count;i++) {
        uint8_t value=animation_byte(c,source);jx_far_set_byte(c,*destination,0,value);destination->offset=(uint16_t)(destination->offset+1);
    }
}
/* 1000:4F59 register ABI: DS:SI input, ES:DI output, BX record index.
 * DF=0. Compressed streams must terminate as required by the original.
 * Forward overlapping copies and fixed-segment offset wrapping are intentional. */
JxAnimationPacketResult jx_animation_packet(JxEventContext *c,JxFarPointer source,JxFarPointer destination,uint16_t record) {
    uint8_t carry=0;
    if(!jx_far_word(c,source.segment,(uint16_t)(2*record+8))) goto finish;
    if(animation_word(c,&source)!=0x42) {carry=1;goto finish;}
    uint16_t encoding=animation_word(c,&source);
    if(encoding==0) {
        for(uint16_t i=0;i<32000;i++) {
            uint16_t value=animation_word(c,&source);
            uint32_t at=((uint32_t)destination.segment*16+destination.offset)&0xfffff;
            c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
            destination.offset=(uint16_t)(destination.offset+2);
        }
        goto finish;
    }
    if(encoding!=1) {carry=1;goto finish;}
    for(;;) {
        uint8_t command=animation_byte(c,&source);
        if(command==0) {
            uint16_t run=animation_word(c,&source);
            animation_fill(c,&destination,(uint8_t)run,(uint8_t)(run>>8));
        } else if(command==0x80) {
            uint16_t run=animation_word(c,&source);
            if(!run) break;
            if(!(run&0x8000)) destination.offset=(uint16_t)(destination.offset+run);
            else if(run&0x4000) {
                uint8_t value=animation_byte(c,&source);animation_fill(c,&destination,(uint16_t)(run&0x3fff),value);
            } else animation_copy(c,&destination,&source,(uint16_t)(run&0x3fff));
        } else if(command&0x80) destination.offset=(uint16_t)(destination.offset+(command&0x7f));
        else animation_copy(c,&destination,&source,command);
    }
finish:
    return (JxAnimationPacketResult){source.offset,destination.offset,carry};
}
static void animation_store_word(JxEventContext *c,JxFarPointer *p,uint16_t value) {
    uint32_t at=((uint32_t)p->segment*16+p->offset)&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
    p->offset=(uint16_t)(p->offset+2);
}
static void animation_field(JxEventContext *c,JxFarPointer p,uint16_t offset,uint16_t value) {
    p.offset=(uint16_t)(p.offset+offset);animation_store_word(c,&p,value);
}
static uint16_t animation_get(JxEventContext *c,JxFarPointer p,uint16_t offset) {
    return jx_far_word(c,p.segment,(uint16_t)(p.offset+offset));
}
/* 1000:4E32. Error values FFFF=signature, FFFE=open, FFFD=allocation.
 * Failed reads and malformed index tables retain original behavior. DF=0. */
uint16_t jx_animation_open(JxEventContext *c,JxFarPointer filename,JxFarPointer state,const JxAnimationServices *s) {
    JxDosWriteResult opened={0,0};s->open(s->opaque,filename,0,&opened);
    if(opened.carry) return 0xfffe;
    uint16_t handle=opened.ax;animation_field(c,state,0,handle);
    JxFarPointer code_flag={0x4e30,0x1000};animation_field(c,code_flag,0,0);
    uint16_t buffer=animation_get(c,state,2);
    if(!buffer) {
        animation_field(c,code_flag,0,1);
        JxDosAllocateResult allocation={0,0};s->allocate(s->opaque,0x1000,&allocation);
        if(allocation.carry) return 0xfffd;
        buffer=allocation.segment;animation_field(c,state,2,buffer);
    }
    JxDosWriteResult read_result={0,0};s->read(s->opaque,handle,(JxFarPointer){0,buffer},0x0b00,&read_result);
    JxFarPointer source={0,buffer};
    if(animation_word(c,&source)!=0x504c || animation_word(c,&source)!=0x2046) return 0xffff;
    source.offset=(uint16_t)(source.offset+2);
    animation_field(c,state,4,animation_word(c,&source));
    animation_field(c,state,6,(uint16_t)(animation_word(c,&source)-1));
    animation_field(c,state,8,jx_far_byte(c,(JxFarPointer){0x44,buffer},0));
    for(uint16_t offset=0x90a;offset<=0x910;offset+=2) animation_field(c,state,offset,0);
    source.offset=0x100;JxFarPointer destination={(uint16_t)(state.offset+10),state.segment};
    for(unsigned i=0;i<256;i++) {
        uint16_t bg=animation_word(c,&source),ra=animation_word(c,&source);
        animation_fill(c,&destination,1,(uint8_t)ra>>2);
        animation_fill(c,&destination,1,(uint8_t)(bg>>8)>>2);
        animation_fill(c,&destination,1,(uint8_t)bg>>2);
    }
    uint16_t table=source.offset,frame=0,outer=animation_get(c,(JxFarPointer){0,buffer},6);
    do {
        source.offset=table;
        uint16_t entries=animation_get(c,(JxFarPointer){0,buffer},6),remaining=entries;
        do {
            if(frame==animation_get(c,source,0)) {
                animation_store_word(c,&destination,(uint16_t)(entries-remaining));
                uint16_t count=animation_get(c,source,2);
                animation_store_word(c,&destination,count);frame=(uint16_t)(frame+count);
                uint16_t size=(uint16_t)(2*count+16+animation_get(c,source,4));
                animation_store_word(c,&destination,size);break;
            }
            source.offset=(uint16_t)(source.offset+6);
        } while(--remaining);
    } while(--outer);
    if(animation_get(c,source,2)>1) {
        JxFarPointer count={(uint16_t)(destination.offset-4),destination.segment};
        animation_field(c,count,0,(uint16_t)(animation_get(c,count,0)-1));
    } else animation_field(c,state,4,(uint16_t)(animation_get(c,state,4)-1));
    return 0;
}
/* 1000:4FE0 reads the ownership flag through the animation state's segment,
 * unlike open's CS override. Preserve that observable original discrepancy. */
void jx_animation_close(JxEventContext *c,JxFarPointer state,const JxAnimationServices *s) {
    JxDosWriteResult closed={0,0};s->close(s->opaque,animation_get(c,state,0),&closed);
    if(jx_far_word(c,state.segment,0x4e30)==1)
        s->free_block(s->opaque,animation_get(c,state,2));
}
static JxFarPointer animation_normalize(JxFarPointer target) {
    target.segment=(uint16_t)(target.segment+(target.offset>>4));target.offset&=15;return target;
}
static void animation_seek_page(const JxAnimationServices *s,uint16_t handle,uint16_t page) {
    JxDosSeekResult result={0,0,0};s->seek(s->opaque,handle,0,((uint32_t)page<<16)|0x0b00,&result);
}
static uint16_t animation_read_page(const JxAnimationServices *s,uint16_t handle,uint16_t buffer,uint16_t size) {
    JxDosWriteResult result={0,0};s->read(s->opaque,handle,(JxFarPointer){0,buffer},size,&result);return result.ax;
}
static uint16_t animation_packet_start(JxEventContext *c,uint16_t buffer) {
    return (uint16_t)(8+2*jx_far_word(c,buffer,2));
}
/* 1000:4C90: first frame, including prefetch of the next file page when
 * the first page holds only one frame. DOS carry and packet carry ignored. */
void jx_animation_first(JxEventContext *c,JxFarPointer state,JxFarPointer target,const JxAnimationServices *s) {
    animation_seek_page(s,animation_get(c,state,0),animation_get(c,state,0x30a));
    uint16_t buffer=animation_get(c,state,2);
    if(!animation_read_page(s,animation_get(c,state,0),buffer,animation_get(c,state,0x30e))) return;
    JxAnimationPacketResult decoded=jx_animation_packet(c,(JxFarPointer){animation_packet_start(c,buffer),buffer},animation_normalize(target),0);
    uint16_t remaining=(uint16_t)(animation_get(c,state,0x30c)-1);
    animation_field(c,state,0x90a,decoded.source_offset);animation_field(c,state,0x90c,1);
    if(remaining) {
        animation_field(c,state,0x90e,1);animation_field(c,state,0x910,0);
    } else {
        animation_field(c,state,0x90e,0);animation_field(c,state,0x910,1);
        uint16_t handle=animation_get(c,state,0);
        animation_seek_page(s,handle,animation_get(c,state,0x310));
        buffer=animation_get(c,state,2);
        if(!animation_read_page(s,handle,buffer,animation_get(c,state,0x314))) return;
        animation_field(c,state,0x90a,animation_packet_start(c,buffer));
    }
}
/* 1000:4D50. The original first-frame tail-call stores its return address
 * in CS:4D7E/4D80; high-level C uses a direct call, without code mutation. */
void jx_animation_next(JxEventContext *c,JxFarPointer state,JxFarPointer target,const JxAnimationServices *s) {
    uint16_t frame=animation_get(c,state,0x90c);
    if(!frame) {jx_animation_first(c,state,target,s);return;}
    if(frame>=animation_get(c,state,6)) return;
    JxAnimationPacketResult decoded=jx_animation_packet(c,
        (JxFarPointer){animation_get(c,state,0x90a),animation_get(c,state,2)},
        animation_normalize(target),animation_get(c,state,0x90e));
    uint16_t index=(uint16_t)(6*animation_get(c,state,0x910));
    uint16_t last=(uint16_t)(animation_get(c,state,(uint16_t)(index+0x30c))-1);
    animation_field(c,state,0x90a,decoded.source_offset);
    animation_field(c,state,0x90c,(uint16_t)(animation_get(c,state,0x90c)+1));
    if(animation_get(c,state,0x90e)<last) {
        animation_field(c,state,0x90e,(uint16_t)(animation_get(c,state,0x90e)+1));
    } else {
        animation_field(c,state,0x90e,0);
        animation_field(c,state,0x910,(uint16_t)(animation_get(c,state,0x910)+1));
        uint16_t size=animation_get(c,state,(uint16_t)(index+0x314));
        uint16_t handle=animation_get(c,state,0);
        animation_seek_page(s,handle,animation_get(c,state,(uint16_t)(index+0x310)));
        uint16_t buffer=animation_get(c,state,2);
        if(!animation_read_page(s,handle,buffer,size)) return;
        animation_field(c,state,0x90a,animation_packet_start(c,buffer));
    }
}
