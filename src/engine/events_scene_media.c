#include "event_engine.h"
/* 1F2A:5970: decode animation frames into four EMS pages per frame. */
uint16_t jx_animation_cache(JxEventContext *c,JxFarPointer filename,uint16_t first_page) {
    for(uint16_t page=0;page<4;page++)
        (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(179+page),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1000,0x4e32,filename.offset,filename.segment,0x3542,c->data_segment);
    for(uint16_t frame=0;frame<jx_word(c,0x3548);frame++) {
        for(uint16_t page=0;page<4;page++)
            (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(179+page),jx_word(c,0x9332));
        (void)JX_CALL(c,0x1000,0x4d50,0x3542,c->data_segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        for(uint16_t page=0;page<4;page++)
            (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(first_page+4*frame+page),jx_word(c,0x9332));
        (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x5858),jx_word(c,0x585a),jx_word(c,0x2eb0),jx_word(c,0x2eb2),64000);
    }
    (void)JX_CALL(c,0x1000,0x4fe0,0x3542,c->data_segment);
    return 0;
}
/* 1F2A:5AE8: rain/snow/light-snow selection; other values do nothing. */
void jx_weather_load(JxEventContext *c) {
    uint16_t weather=jx_word(c,0x9364),name;
    switch(weather) {
        case 1:name=0x02aa;break;
        case 2:name=0x02b3;break;
        case 3:name=0x02bc;break;
        default:return;
    }
    (void)JX_CALL(c,0x1f2a,0x5970,name,c->data_segment,87);
}
/* 1F2A:5FD8. Explicit 72-byte workspace retains original formatting bytes.
 * Sprintf uses the original live format at DS:03F9, followed by the original
 * four-byte space replacement/truncation, including any untouched bytes. */
void jx_sound_resource_load(JxEventContext *c,uint16_t resource,uint16_t channel,JxFarPointer work) {
    if(!jx_word(c,0x31b6) || jx_signed_word(resource)<0) return;
    (void)JX_CALL(c,0x1000,0x2a8c,work.offset,work.segment,0x03f9,c->data_segment,resource);
    for(uint16_t i=0;i<4;i++)
        if(jx_far_byte(c,work,i)==32) jx_far_set_byte(c,work,i,48);
    jx_far_set_byte(c,work,4,0);
    JxFarPointer filename={(uint16_t)(work.offset+6),work.segment};
    (void)jx_string_copy(c,filename,(JxFarPointer){0x03fd,c->data_segment});
    (void)jx_string_append(c,filename,work);
    (void)jx_string_append(c,filename,(JxFarPointer){0x0404,c->data_segment});
    (void)JX_CALL(c,0x1f2a,0x1d6e,filename.offset,filename.segment,channel);
}
