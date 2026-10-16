#include "event_engine.h"
static uint16_t heap_read(JxEventContext *c,JxFarPointer p,uint16_t n) {
    return jx_far_word(c,p.segment,(uint16_t)(p.offset+n));
}
static void heap_write(JxEventContext *c,JxFarPointer p,uint16_t n,uint16_t v) {
    uint32_t a=((uint32_t)p.segment*16+(uint16_t)(p.offset+n))&0xfffff;
    c->memory[a]=(uint8_t)v;c->memory[(a+1)&0xfffff]=(uint8_t)(v>>8);
}
/* 1000:4B3A. Header +20 is the initial free block, end-2 the sentinel. */
void jx_heap_initialize(JxEventContext *c,JxFarPointer heap,uint16_t size) {
    uint16_t end=(uint16_t)(heap.offset+size),first=(uint16_t)(heap.offset+20);
    heap_write(c,heap,4,end);
    heap_write(c,(JxFarPointer){(uint16_t)(end-2),heap.segment},0,0xfffe);
    heap_write(c,heap,10,(uint16_t)(end-2));
    heap_write(c,(JxFarPointer){first,heap.segment},0,(uint16_t)(size-23));
    heap_write(c,heap,0,heap.segment);
    heap_write(c,heap,6,first);heap_write(c,heap,8,first);
    for(uint16_t n=12;n<20;n+=2) heap_write(c,heap,n,0);
}
/* 1000:4B6E. Append heap to descriptor's first/current/last far-pointer list. */
void jx_heap_link(JxEventContext *c,JxFarPointer heap,JxFarPointer descriptor) {
    if(!heap_read(c,descriptor,2)) {
        heap_write(c,descriptor,2,heap.segment);heap_write(c,descriptor,0,heap.offset);
    } else {
        JxFarPointer last={heap_read(c,descriptor,8),heap_read(c,descriptor,10)};
        heap_write(c,last,14,heap.segment);heap_write(c,last,12,heap.offset);
        heap_write(c,heap,18,last.segment);heap_write(c,heap,16,last.offset);
    }
    heap_write(c,descriptor,10,heap.segment);heap_write(c,descriptor,8,heap.offset);
    heap_write(c,descriptor,6,heap.segment);heap_write(c,descriptor,4,heap.offset);
}
/* 1000:4BA4. Next-fit scan, merging adjacent free blocks in place. */
JxHeapBlockResult jx_heap_take_block(JxEventContext *c,JxFarPointer heap,uint16_t size) {
    uint16_t need=(uint16_t)((size+1)&0xfffe),cursor=heap_read(c,heap,8),limit=heap_read(c,heap,10);
    int wrapped=0;
    for(;;) {
        uint16_t header=cursor,block=heap_read(c,(JxFarPointer){cursor,heap.segment},0);
        cursor=(uint16_t)(cursor+2);
        if(block&1) {
            uint16_t data=cursor;
            for(;;) {
                uint16_t available=(uint16_t)(block-1);
                if(available>=need) {
                    heap_write(c,(JxFarPointer){(uint16_t)(data-2),heap.segment},0,need);
                    if(available!=need) heap_write(c,(JxFarPointer){(uint16_t)(data+need),heap.segment},0,(uint16_t)(available-need-1));
                    heap_write(c,heap,8,(uint16_t)(data+need));
                    return (JxHeapBlockResult){{data,heap.segment},need,0};
                }
                if((uint32_t)cursor+available>65535) goto failed;
                cursor=(uint16_t)(cursor+available);header=cursor;
                uint16_t next=heap_read(c,(JxFarPointer){cursor,heap.segment},0);cursor=(uint16_t)(cursor+2);
                if(!(next&1)) {block=next;break;}
                block=(uint16_t)(next+available+2);cursor=data;
                heap_write(c,(JxFarPointer){(uint16_t)(data-2),heap.segment},0,block);
            }
        }
        if(header>=limit) {
            if(wrapped || (limit&1)) break;
            uint16_t first=heap_read(c,heap,6),previous=heap_read(c,heap,8);
            if(first==previous) break;
            cursor=first;limit=(uint16_t)(previous-1);wrapped=1;
        } else {
            if((uint32_t)cursor+block>65535) break;
            cursor=(uint16_t)(cursor+block);
        }
    }
failed:
    heap_write(c,heap,8,heap_read(c,heap,6));
    return (JxHeapBlockResult){{0,0},need,1};
}

/* 1000:4C20. Allocate whole DOS paragraphs, skipping segments at or below
 * the configured lower bound exactly as the original does (no implicit free). */
JxHeapGrowResult jx_heap_grow(JxEventContext *c,JxFarPointer descriptor,
    uint16_t size,JxDosAllocateService service,void *opaque) {
    uint16_t bytes=(uint16_t)((size+39)&0xfff0);
    uint16_t paragraphs=bytes?(uint16_t)(bytes>>4):4096;
    JxDosAllocateResult result;
    do {
        result=(JxDosAllocateResult){0,0};service(opaque,paragraphs,&result);
        if(result.carry) return (JxHeapGrowResult){{0,0},1};
    } while(result.segment<=jx_word(c,0x12ec));
    if(result.segment>jx_word(c,0x12ea)) jx_set_word(c,0x12ea,result.segment);
    JxFarPointer heap={0,result.segment};
    heap_write(c,heap,2,heap_read(c,descriptor,12));
    jx_heap_initialize(c,heap,bytes);jx_heap_link(c,heap,descriptor);
    return (JxHeapGrowResult){heap,0};
}

/* 1000:23D4: last real block before FFFE sentinel, starting at cached cursor. */
uint16_t jx_heap_tail(JxEventContext *c,JxFarPointer heap) {
    uint16_t cursor=heap_read(c,heap,8),last=cursor;
    if(cursor==heap_read(c,heap,10)) cursor=heap_read(c,heap,6);
    for(;;) {
        uint16_t block=heap_read(c,(JxFarPointer){cursor,heap.segment},0);
        if(block==0xfffe) return last;
        last=cursor;cursor=(uint16_t)(cursor+2+(block&0xfffe));
    }
}
/* 1000:2383. Return carry as failed; resize within pre-reserved arena can
 * succeed without DOS. DOS service preserves all registers other than AX/BX. */
uint8_t jx_heap_resize(JxEventContext *c,JxFarPointer heap,uint16_t size,
    JxDosResizeService service,void *opaque) {
    if(heap_read(c,heap,2)&4) {
        uint16_t last=(uint16_t)(size-1),current=(uint16_t)(heap_read(c,heap,4)-1);
        if(last>current && heap_read(c,heap,0xfffe)>=last) return 0;
    }
    uint16_t paragraphs=(uint16_t)(size>>4),segment=heap.segment;
    if(!paragraphs) paragraphs=4096;
    if(heap_read(c,heap,2)&4) {
        segment=jx_far_word(c,heap.segment,0x1312);
        paragraphs=(uint16_t)(paragraphs+heap.segment-segment);
    }
    if(service(opaque,segment,paragraphs)) return 1;
    if(heap_read(c,heap,2)&4) heap_write(c,heap,0xfffe,(uint16_t)(size-1));
    return 0;
}
/* 1000:22F4, indirect second-pass allocation helper. */
uint8_t jx_heap_expand(JxEventContext *c,JxFarPointer heap,uint16_t request,
    JxDosResizeService service,void *opaque) {
    if(!(heap_read(c,heap,2)&1)) return 1;
    uint16_t tail=jx_heap_tail(c,heap),block=heap_read(c,(JxFarPointer){tail,heap.segment},0);
    if(block&1) request=(uint16_t)(request-block-1);
    request=(uint16_t)(request+2);
    uint16_t old_size=heap_read(c,heap,4);
    if(!old_size) return 1;
    uint32_t sum=(uint32_t)request+old_size;
    uint16_t target=(uint16_t)sum,alignment,mask,size;
    if(sum>65535) {
        if(target) return 1;
        size=0;mask=0xfff0;
    } else {
        alignment=jx_word(c,0x15d2);
        if(alignment!=0x2000) {
            uint16_t lower=0x8000;
            while(lower && lower>=alignment) lower>>=1;
            alignment=lower>=8?(uint16_t)(lower<<1):16;
        }
        uint16_t extra=(uint16_t)(alignment-1);
        mask=(uint16_t)~extra;
        uint32_t rounded=(uint32_t)extra+target;
        size=rounded>65535?0:(uint16_t)(rounded&mask);
    }
    if(jx_heap_resize(c,heap,size,service,opaque)) {
        if(mask==0xfff0) return 1;
        uint32_t rounded=(uint32_t)target+15;
        size=rounded>65535?0:(uint16_t)(rounded&0xfff0);
        if(jx_heap_resize(c,heap,size,service,opaque)) return 1;
    }
    uint16_t extension=(uint16_t)(size-heap_read(c,heap,4));
    heap_write(c,heap,4,size);heap_write(c,heap,8,tail);
    uint16_t sentinel=heap_read(c,heap,10);
    heap_write(c,(JxFarPointer){sentinel,heap.segment},0,(uint16_t)(extension-1));
    sentinel=(uint16_t)(sentinel+extension);
    heap_write(c,(JxFarPointer){sentinel,heap.segment},0,0xfffe);
    heap_write(c,heap,10,sentinel);return 0;
}

static JxFarPointer heap_global(JxEventContext *c,uint16_t offset) {
    return (JxFarPointer){jx_word(c,offset),jx_word(c,(uint16_t)(offset+2))};
}
/* 1000:4AA1. Search cached->last then first->previous, repeat with expansion,
 * then obtain a new DOS heap. Failure handler may repair state and retry. */
uint32_t jx_malloc(JxEventContext *c,uint16_t request,JxDosAllocateService allocate,
    JxDosResizeService resize,void *opaque) {
    for(;;) {
        uint16_t size=request;
        JxFarPointer heap={0,0};JxHeapBlockResult block={0};
        if(size<=0xffe8) {
            if(jx_word(c,0x1816)) {
                for(unsigned expand=0;expand<2;expand++) {
                    uint16_t last_segment=jx_word(c,0x181e);
                    heap=heap_global(c,0x1818);
                    for(;;) {
                        uint16_t start_segment=heap.segment;
                        for(;;) {
                            if(!expand) {
                                block=jx_heap_take_block(c,heap,size);size=block.rounded_size;
                                if(!block.failed) goto allocated;
                            } else if(!jx_heap_expand(c,heap,size,resize,opaque)) {
                                block=jx_heap_take_block(c,heap,size);size=block.rounded_size;
                                goto allocated;
                            }
                            uint16_t old_segment=heap.segment;
                            heap=(JxFarPointer){heap_read(c,heap,12),heap_read(c,heap,14)};
                            if(old_segment==last_segment) break;
                        }
                        JxFarPointer current=heap_global(c,0x1818);
                        last_segment=heap_read(c,current,18);
                        heap=heap_global(c,0x1814);
                        if(heap.segment==start_segment) break;
                    }
                }
            }
            JxHeapGrowResult grown=jx_heap_grow(c,(JxFarPointer){0x1814,c->data_segment},size,allocate,opaque);
            if(!grown.failed) {
                heap=grown.heap;block=jx_heap_take_block(c,heap,size);goto allocated;
            }
        }
        {
            JxFarPointer handler=heap_global(c,0x1810);
            if(!handler.offset && !handler.segment) return 0;
            if(!(uint16_t)JX_CALL(c,handler.segment,handler.offset,request)) return 0;
        }
        continue;
allocated:
        /* Valid heaps guarantee success after successful expansion/growth. */
        jx_set_word(c,0x181a,block.pointer.segment);jx_set_word(c,0x1818,heap.offset);
        return block.pointer.offset|((uint32_t)block.pointer.segment<<16);
    }
}

/* 1000:4A8E. Null tests segment only; free merely marks block low bit. */
void jx_free(JxEventContext *c,JxFarPointer pointer) {
    if(pointer.segment) {
        pointer.offset=(uint16_t)(pointer.offset-2);
        jx_far_set_byte(c,pointer,0,(uint8_t)(jx_far_byte(c,pointer,0)|1));
    }
}
