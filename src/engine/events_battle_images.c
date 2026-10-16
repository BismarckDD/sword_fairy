#include "event_engine.h"

static JxFarPointer image_local(JxFarPointer work,uint16_t displacement) {
    work.offset=(uint16_t)(work.offset+displacement);return work;
}
/* The original private frame has 74 bytes: number[4], FILE far pointer,
 * and filename[66]. Preserve its two-character truncation, even for >=100. */
static uint32_t battle_image_open(JxEventContext *c,uint16_t number,
                                 JxFarPointer work,int background) {
    JxFarPointer filename=image_local(work,8);
    (void)JX_CALL(c,0x1000,0x2a8c,work.offset,work.segment,
                  background?0x0147:0x015d,c->data_segment,number);
    if(jx_far_byte(c,work,0)==0x20)jx_far_set_byte(c,work,0,0x30);
    jx_far_set_byte(c,work,2,0);
    JxFarPointer prefix={background?0x014b:0x0161,c->data_segment};
    JxFarPointer suffix={background?0x0155:0x016e,c->data_segment};
    (void)jx_string_copy(c,filename,prefix);
    (void)jx_string_append(c,filename,work);
    (void)jx_string_append(c,filename,suffix);
    uint32_t file=JX_CALL(c,0x1000,0x0738,filename.offset,filename.segment,
                          background?0x015a:0x0173,c->data_segment);
    for(unsigned i=0;i<4;i++)jx_far_set_byte(c,work,(uint16_t)(4+i),(uint8_t)(file>>(8*i)));
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),0x10,0,0);
    return file;
}
/* 1F2A:46E6. Read 320x200 pixels and 255 RGB entries; the final palette
 * entry is supplied from 64DB. The original does not check fopen/fread. */
void jx_battle_load_background(JxEventContext *c,uint16_t number,JxFarPointer work) {
    if(jx_signed_word(number)<0)return;
    uint32_t file=battle_image_open(c,number,work,1);
    for(uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(135+slot),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,64000,
                  (uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x0754,0x2eb4,c->data_segment,3,255,
                  (uint16_t)file,(uint16_t)(file>>16));
    jx_set_word(c,0x31b1,jx_word(c,0x64db));
    jx_ds(c)[0x31b3]=jx_ds(c)[0x64dd];
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
}
/* 1F2A:4830. Two EMS pages hold the 220x148 actor image. */
void jx_battle_load_actor_image(JxEventContext *c,uint16_t page,uint16_t number,JxFarPointer work) {
    if(jx_signed_word(number)<0)return;
    uint32_t file=battle_image_open(c,number,work,0);
    for(uint16_t slot=0;slot<2;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(page+slot),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,32560,
                  (uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
}
