#include "event_engine.h"
static uint16_t world_track_duration(JxEventContext *c) {
    uint16_t offset=(uint16_t)(0x0510+2u*jx_word(c,0x889a));
    return jx_far_word(c,jx_word(c,0x23ac),offset);
}
static void world_play_track(JxEventContext *c) {
    uint16_t duration=world_track_duration(c);
    uint16_t track=(uint16_t)((jx_ds(c)[0x932c]+jx_word(c,0x889a))&255);
    (void)JX_CALL(c,0x1832,0x224c,track,2,duration);
}
/* 2F28:A0FC. Entry gates are not rechecked after device callbacks. The
 * duration table segment/index, base track and status remain live. Elapsed
 * minutes*60+seconds is compared as signed word to the table duration. */
void jx_world_cd_update(JxEventContext *c) {
    if(jx_signed_word(jx_word(c,0x889a))<0 || jx_word(c,0x050c)==0)return;
    if((uint16_t)JX_CALL(c,0x1832,0x2212,0x933e,c->data_segment)!=0) {
        world_play_track(c);return;
    }
    uint16_t elapsed=(uint16_t)(jx_ds(c)[0x9340]*60u+jx_ds(c)[0x9341]);
    if(jx_signed_word(elapsed)>=jx_signed_word(world_track_duration(c))) {
        world_play_track(c);return;
    }
    (void)JX_CALL(c,0x1832,0x2180,0x8738,c->data_segment);
    if(jx_word(c,0x8738)!=0)world_play_track(c);
}
