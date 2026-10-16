#include "event_engine.h"
/* 1F2A:6496. PLACE.BIN records are 0xAE0 bytes: a 64-byte header,
 * fifty 16-byte exits, and thirty 64-byte NPC records. Only the first
 * 54 header bytes and first 60 bytes of each NPC record are consumed. */
uint16_t jx_place_load(JxEventContext *c,uint16_t place) {
    uint32_t stream=JX_CALL(c,0x1000,0x0738,0x0460,c->data_segment,0x045d,c->data_segment);
    if(!stream) return 0xffff;
    uint16_t stream_offset=(uint16_t)stream,stream_segment=(uint16_t)(stream>>16);
    uint32_t position=(uint32_t)(jx_signed_word(place)*2784);
    (void)JX_CALL(c,0x1000,0x27fa,stream_offset,stream_segment,(uint16_t)position,(uint16_t)(position>>16),0);
    (void)JX_CALL(c,0x1000,0x0754,0x9348,c->data_segment,1,54,stream_offset,stream_segment);
    (void)JX_CALL(c,0x1000,0x27fa,stream_offset,stream_segment,10,0,1);
    (void)JX_CALL(c,0x1000,0x0754,0x3e56,c->data_segment,16,50,stream_offset,stream_segment);
    for(uint16_t i=0;i<30;i++) {
        (void)JX_CALL(c,0x1000,0x0754,(uint16_t)(0x889c+86*i),c->data_segment,1,60,stream_offset,stream_segment);
        (void)JX_CALL(c,0x1000,0x27fa,stream_offset,stream_segment,4,0,1);
    }
    (void)JX_CALL(c,0x1000,0x0636,stream_offset,stream_segment);
    if(!jx_word(c,0xb684)) for(uint16_t i=0;i<30;i++) {
        uint16_t actor=(uint16_t)(0x889c+86*i);
        for(uint16_t field=0;field<6;field+=2)
            jx_set_word(c,(uint16_t)(actor+0x40+field),jx_word(c,(uint16_t)(actor+0x14+field)));
        jx_set_word(c,(uint16_t)(actor+0x3c),0);
    }
    return 0;
}
/* 1F2A:5230. Caller supplies private82-byte storage: filename[32] and
 * member[50]. Valid inputs must fit the original local buffers. Index
 * short reads leave old table bytes intact; scan still checks400 entries. */
uint32_t jx_archive_open(JxEventContext *c,JxFarPointer archive,JxFarPointer member,JxFarPointer work) {
    JxFarPointer name={(uint16_t)(work.offset+32),work.segment};
    (void)jx_string_copy(c,name,member);(void)jx_string_uppercase(c,name);
    for(uint16_t i=0;i<12;i++) if(jx_far_byte(c,name,i)==' ') jx_far_set_byte(c,name,i,0);
    (void)jx_string_copy(c,work,archive);
    (void)jx_string_append(c,work,(JxFarPointer){0x0253,c->data_segment});
    uint32_t stream=JX_CALL(c,0x1000,0x0738,work.offset,work.segment,0x025d,c->data_segment);
    if(!stream) return 0;
    (void)JX_CALL(c,0x1000,0x0754,0x93aa,c->data_segment,16,400,(uint16_t)stream,(uint16_t)(stream>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
    uint32_t position=UINT32_MAX;
    for(uint16_t i=0;i<400;i++) {
        uint16_t entry=(uint16_t)(0x93aa+16*i);
        if(jx_string_compare(c,(JxFarPointer){entry,c->data_segment},name)==0) {
            position=(uint32_t)jx_word(c,(uint16_t)(entry+12))|((uint32_t)jx_word(c,(uint16_t)(entry+14))<<16);break;
        }
    }
    if(position&UINT32_C(0x80000000)) return 0;
    (void)jx_string_copy(c,work,archive);
    (void)jx_string_append(c,work,(JxFarPointer){0x0258,c->data_segment});
    stream=JX_CALL(c,0x1000,0x0738,work.offset,work.segment,0x0260,c->data_segment);
    if(!stream) return 0;
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)stream,(uint16_t)(stream>>16),(uint16_t)position,(uint16_t)(position>>16),0);
    return stream;
}
/* 1F2A:640E: ten 10-byte scene objects per signed place index.
 * Successful path returns fclose's AX, including close failure. */
uint16_t jx_scene_objects_load(JxEventContext *c,uint16_t place) {
    uint32_t stream=JX_CALL(c,0x1000,0x0738,0x0455,c->data_segment,0x0452,c->data_segment);
    if(!stream) return 0xffff;
    uint32_t offset=(uint32_t)((int32_t)jx_signed_word(place)*100);
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)stream,(uint16_t)(stream>>16),(uint16_t)offset,(uint16_t)(offset>>16),0);
    (void)JX_CALL(c,0x1000,0x0754,0x92b8,c->data_segment,10,10,(uint16_t)stream,(uint16_t)(stream>>16));
    return (uint16_t)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
}
/* 1F2A:65FC. Private filename workspace is 18 bytes. */
void jx_scene_backdrop_load(JxEventContext *c,JxFarPointer filename) {
    uint16_t mode=jx_word(c,0x9362);
    uint16_t name=mode==1?0x046a:mode==2?0x0473:0x047c;
    (void)jx_string_copy(c,filename,(JxFarPointer){name,c->data_segment});
    uint32_t stream=JX_CALL(c,0x1000,0x0738,filename.offset,filename.segment,0x0483,c->data_segment);
    if(!stream) return;
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)stream,(uint16_t)(stream>>16),16,0,0);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,64000,(uint16_t)stream,(uint16_t)(stream>>16));
    (void)JX_CALL(c,0x1832,0x1810,3);
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,192,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    (void)JX_CALL(c,0x1832,0x1810,jx_word(c,0x3e54));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
}
