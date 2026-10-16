#include "event_engine.h"
#include <string.h>
static uint16_t load_read(JxEventContext *c,JxFarPointer destination,uint16_t size,uint16_t count,uint32_t stream) {
    return (uint16_t)JX_CALL(c,0x1000,0x0754,destination.offset,destination.segment,size,count,(uint16_t)stream,(uint16_t)(stream>>16));
}
static void load_close(JxEventContext *c,uint32_t stream) {(void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));}
static uint32_t load_open(JxEventContext *c,uint16_t archive,uint16_t member,JxFarPointer filename) {
    (void)jx_string_copy(c,filename,(JxFarPointer){member,c->data_segment});
    return JX_CALL(c,0x1f2a,0x5230,archive,c->data_segment,filename.offset,filename.segment);
}
static JxMapLoadFlow load_fatal(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0xb196);return JX_MAP_LOAD_FATAL;
}
/* 1F2A:6700. Explicit48-byte filename workspace replaces its stack buffer.
 * Fatal B196 requests game termination; the typed result prevents a host
 * callback returning from accidentally continuing past that boundary.
 * Palette weight conversion models the original finite nearest-rounding
 * double-to-float path, not arbitrary MS software-FP control modes. */
JxMapLoadFlow jx_map_load(JxEventContext *c,uint16_t place,uint16_t transition,JxFarPointer filename) {
    (void)JX_CALL(c,0x1f2a,0xc7f2);(void)JX_CALL(c,0x1f2a,0xc9a0);
    jx_set_word(c,0x873e,0x86ba);jx_set_word(c,0x8740,0x2f28);
    for(uint16_t i=0;i<10;i++) {
        uint16_t event=(uint16_t)(0x6584+8*i);
        jx_set_word(c,event,999);jx_set_word(c,(uint16_t)(event+2),999);
        jx_set_word(c,(uint16_t)(event+4),0x86ba);jx_set_word(c,(uint16_t)(event+6),0x2f28);
    }
    if((uint16_t)JX_CALL(c,0x1f2a,0x6496,place)) return load_fatal(c);
    for(uint16_t i=0;i<3;i++) (void)JX_CALL(c,0x1f2a,0x5fd8,jx_word(c,(uint16_t)(0x936c+6*i)),(uint16_t)(3+i));
    (void)JX_CALL(c,0x1f2a,0x5ae8);
    uint32_t stream=load_open(c,0x0486,0x9356,filename);
    if(!stream) return load_fatal(c);
    (void)load_read(c,(JxFarPointer){0x663a,c->data_segment},28,1,stream);
    for(uint16_t layer=0;layer<2;layer++) {
        for(uint16_t page=0;page<4;page++)
            (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)((layer?83:63)+page),jx_word(c,0x9332));
        uint16_t count=(uint16_t)((uint32_t)jx_word(c,0x663a)*jx_word(c,0x663c));
        (void)load_read(c,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)},2,count,stream);
    }
    load_close(c,stream);
    stream=load_open(c,0x048a,0x664a,filename);
    if(!stream) return load_fatal(c);
    (void)load_read(c,(JxFarPointer){0x6624,c->data_segment},22,1,stream);
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)stream,(uint16_t)(stream>>16),480,0,1);
    (void)load_read(c,(JxFarPointer){0x63be,c->data_segment},3,95,stream);
    load_close(c,stream);
    uint64_t bits=0;for(unsigned i=0;i<8;i++) bits|=(uint64_t)jx_ds(c)[0x238c+i]<<(8*i);
    double weight;memcpy(&weight,&bits,sizeof weight);float rounded=(float)weight;uint32_t packed;memcpy(&packed,&rounded,sizeof packed);
    (void)JX_CALL(c,0x2f28,0xa8d0,0x61de,c->data_segment,jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,(uint16_t)packed,(uint16_t)(packed>>16));
    stream=load_open(c,0x048e,0x663e,filename);
    if(!stream) return load_fatal(c);
    (void)load_read(c,(JxFarPointer){0x31c4,c->data_segment},6,1,stream);
    int full_pages=jx_signed_word(jx_word(c,0x31c4))/64;
    uint16_t remainder=(uint16_t)(jx_word(c,0x31c4)-(uint16_t)(full_pages*64));
    for(int logical=18;logical<18+full_pages;logical++) {
        (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)logical,jx_word(c,0x9332));
        (void)load_read(c,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)},1,16384,stream);
    }
    (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)(18+full_pages),jx_word(c,0x9332));
    (void)load_read(c,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)},256,remainder,stream);
    load_close(c,stream);
    (void)JX_CALL(c,0x1f2a,0xc6a8);(void)JX_CALL(c,0x1f2a,0x640e,place);
    if(jx_word(c,0x9362)) (void)JX_CALL(c,0x1f2a,0x65fc);
    if(jx_ds(c)[0xb6a3]) (void)JX_CALL(c,0x2f28,0x5580);
    if(!transition) (void)JX_CALL(c,0x150f,0x0006,0x61de,c->data_segment,0,256);
    (void)JX_CALL(c,0x1f2a,0xc24e);
    uint16_t entry=(uint16_t)(0x0a6c+4*jx_word(c,0x9334));
    (void)JX_CALL(c,jx_word(c,(uint16_t)(entry+2)),jx_word(c,entry));
    jx_set_word(c,0x5860,jx_signed_word(jx_word(c,0xd0fa))>0?3:1);
    return JX_MAP_LOAD_RETURNED;
}
