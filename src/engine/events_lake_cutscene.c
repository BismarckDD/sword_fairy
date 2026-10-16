#include "event_engine.h"

static uint16_t lake_next_page(JxEventContext *c) {
    return (uint16_t)((int16_t)(uint16_t)(jx_word(c,0x3e54)+1)%2);
}
static uint32_t lake_open(JxEventContext *c,uint16_t name,uint16_t mode) {
    return JX_CALL(c,0x1000,0x738,name,c->data_segment,mode,c->data_segment);
}
static void lake_read_image(JxEventContext *c,uint32_t file,uint16_t offset,uint16_t segment) {
    (void)JX_CALL(c,0x1000,0x754,offset,segment,1,64000,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x754,0x2eb4,c->data_segment,3,256,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
}
static void lake_seek_image(JxEventContext *c,uint32_t file) {
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),16,0,0);
}
static void lake_map(JxEventContext *c,uint16_t first,uint16_t count) {
    for (uint16_t slot=0;slot<count;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,first+slot,jx_word(c,0x9332));
}
static void lake_fade(JxEventContext *c,uint16_t function,uint16_t palette) {
    (void)JX_CALL(c,0x2f28,function,palette,c->data_segment,1);
}
/* 2F28:373E. event_work owns eight bytes, disjoint from game state/buffers.
 * Only input type 8 advances time. Other events, including keys, are ignored
 * by the original. Input availability and scheduling remain host services. */
void jx_lake_cutscene(JxEventContext *c,JxFarPointer event_work) {
    uint16_t ticks=0,first_frame=0,second_frame=0;
    uint32_t file=lake_open(c,0x0a14,0x0a11);
    if (!file) return;
    jx_set_word(c,0x889a,0xffff);
    if ((int16_t)jx_word(c,0x31b4)>0 && (int16_t)jx_word(c,0x050c)>0) {
        uint16_t seconds=jx_far_word(c,jx_word(c,0x23a0),0x524);
        (void)JX_CALL(c,0x1832,0x224c,(uint8_t)(jx_ds(c)[0x932c]+10),2,seconds);
    }
    lake_seek_image(c,file);lake_map(c,95,4);
    lake_read_image(c,file,jx_word(c,0x5858),jx_word(c,0x585a));
    lake_fade(c,0x3f66,0x61de);
    (void)JX_CALL(c,0x1832,0x1810,lake_next_page(c));
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    (void)JX_CALL(c,0x1f2a,0x288a,0xa1d,c->data_segment,87,1,1,160,144);
    (void)JX_CALL(c,0x1f2a,0x288a,0xa29,c->data_segment,91,1,1,64,16);
    lake_map(c,91,2);
    (void)JX_CALL(c,0x1832,0x1a28,0,50,64,16,jx_word(c,0x5858),jx_word(c,0x585a),255);
    lake_map(c,87,2);
    (void)JX_CALL(c,0x1832,0x1a28,160,56,160,144,jx_word(c,0x5858),jx_word(c,0x585a),255);
    (void)JX_CALL(c,0x1832,0x1836,lake_next_page(c));
    lake_fade(c,0x415e,0x2eb4);
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for (;;) {
        while ((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event_work.offset,event_work.segment)!=0) {}
        if (jx_far_word(c,event_work.segment,event_work.offset)!=8) continue;
        ticks++;
        if (ticks>2120) {
            file=lake_open(c,0xa38,0xa35);
            if (!file) {lake_fade(c,0x3f66,0x2eb4);return;}
            lake_seek_image(c,file);
            lake_read_image(c,file,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
            lake_fade(c,0x3f66,0x2eb4);
            (void)JX_CALL(c,0x1832,0x1a28,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2),255);
            lake_fade(c,0x415e,0x2eb4);
            (void)JX_CALL0(c,0x1f2a,0x7272);
            lake_fade(c,0x3f66,0x2eb4);return;
        }
        if (ticks%2==0) {
            uint8_t saved[3];
            for (uint16_t channel=0;channel<3;channel++) saved[channel]=jx_ds(c)[0x30c1+channel];
            for (uint16_t color=175;color>160;color--)
                for (uint16_t channel=0;channel<3;channel++)
                    jx_ds(c)[0x2eb4+3*color+channel]=jx_ds(c)[0x2eb4+3*(color-1)+channel];
            for (uint16_t channel=0;channel<3;channel++) jx_ds(c)[0x3094+channel]=saved[channel];
            (void)JX_CALL(c,0x150f,6,0x3094,c->data_segment,160,16);
        }
        if (ticks%9==0) {
            first_frame++;
            jx_scene_composite_frame(c,first_frame,first_frame,second_frame);
        } else if (ticks%6==0) {
            second_frame++;
            jx_scene_composite_frame(c,first_frame,first_frame,second_frame);
        }
    }
}
