#include "event_engine.h"
static uint16_t dice_header(JxEventContext *c,JxFarPointer header,uint16_t off) {
    return jx_far_word(c,header.segment,(uint16_t)(header.offset+off));
}
static JxFarPointer dice_background(JxEventContext *c,uint16_t advance) {
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+advance;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+((sum>>16)<<12))};
}
static uint16_t dice_next_page(JxEventContext *c) {
    return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x2952)+1))%2+2);
}
static void dice_face(JxEventContext *c,uint16_t die,uint16_t x,uint16_t y) {
    (void)JX_CALL(c,0x1832,0x1a28,x,y,16,20,(uint16_t)(jx_word(c,0x5858)+die*320u),jx_word(c,0x585a),255);
}
/* 1F2A:B21A.8-byte FLM header: width,height,signed frame count,reserved.
 * Preload mapping statuses and short reads are ignored by the original;
 * playback's face/background mappings91..94 are checked individually. */
void jx_dice_play_animation(JxEventContext *c,JxFarPointer filename,uint16_t first,uint16_t second,uint16_t third,uint16_t faces,JxFarPointer header) {
    uint32_t file=JX_CALL(c,0x1000,0x0738,filename.offset,filename.segment,0x074e,c->data_segment);
    if(!file)return;
    (void)JX_CALL(c,0x1000,0x0754,header.offset,header.segment,1,8,(uint16_t)file,(uint16_t)(file>>16));
    for(uint16_t frame=0;jx_signed_word(frame)<jx_signed_word(dice_header(c,header,4));frame++) {
        (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)(95+frame),jx_word(c,0x9332));
        uint16_t size=(uint16_t)((uint32_t)dice_header(c,header,0)*dice_header(c,header,2));
        (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,size,(uint16_t)file,(uint16_t)(file>>16));
    }
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
    for(uint16_t frame=0;jx_signed_word(frame)<jx_signed_word(dice_header(c,header,4));frame++) {
        (void)JX_CALL(c,0x1832,0x1810,dice_next_page(c));
        JxFarPointer background=dice_background(c,faces?12000:30000);
        (void)JX_CALL(c,0x1832,0x1c6c,72,0,180,100,background.offset,background.segment);
        if(faces) {
            if((uint16_t)JX_CALL(c,0x1503,0x007d,0,87,jx_word(c,0x9332)))return;
            dice_face(c,first,147,60);dice_face(c,second,139,75);dice_face(c,third,155,75);
        }
        for(uint16_t page=0;page<4;page++)
            if((uint16_t)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(91+page),jx_word(c,0x9332)))return;
        (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)(95+frame),jx_word(c,0x9332));
        (void)JX_CALL(c,0x1832,0x1a28,80,0,dice_header(c,header,0),dice_header(c,header,2),jx_word(c,0x5858),jx_word(c,0x585a),255);
        (void)JX_CALL(c,0x1832,0x1836,dice_next_page(c));
        jx_set_word(c,0x2952,(uint16_t)(jx_word(c,0x2952)+1));
        (void)JX_CALL(c,0x1f2a,0x2dbc,2);
    }
}
