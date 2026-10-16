#include "event_engine.h"

/* 1F2A:9D34. Configuration gate is nonzero, not signed positive. */
void jx_music_toggle(JxEventContext *c) {
    if(jx_word(c,0x31b4)==0)return;
    uint16_t enabled=(uint16_t)(1-jx_word(c,0x050c));jx_set_word(c,0x050c,enabled);
    if(enabled==1)(void)JX_CALL0(c,0x1832,0x2324);
    else (void)JX_CALL0(c,0x1832,0x22f2);
}
/* 1F2A:9D6C. All word values preserve the original wrapping subtraction. */
void jx_sound_toggle(JxEventContext *c) {jx_set_word(c,0x050e,(uint16_t)(1-jx_word(c,0x050e)));}
/* 1F2A:9D80. This hotkey uses signed-positive gates and the DS table. */
void jx_music_restart(JxEventContext *c) {
    if(jx_signed_word(jx_word(c,0x31b4))<=0 || jx_signed_word(jx_word(c,0x050c))<=0)return;
    uint16_t track=jx_word(c,0x889a),duration=jx_word(c,(uint16_t)(0x0510+2*track));
    (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track)&255),2,duration);
}

/* 2F28:856C, called on timer events. This path uses DS:23AC for the
 * resource table; scene-start music calls use DS:23AE instead. */
void jx_music_poll(JxEventContext *c) {
    if(jx_signed_word(jx_word(c,0x889a))<0 || jx_word(c,0x050c)==0) return;
    int restart=(uint16_t)JX_CALL(c,0x1832,0x2212,0x933e,c->data_segment)!=0;
    if(!restart) {
        uint16_t elapsed=(uint16_t)(jx_ds(c)[0x9340]*60u+jx_ds(c)[0x9341]);
        uint16_t track=jx_word(c,0x889a);
        uint16_t duration=jx_far_word(c,jx_word(c,0x23ac),(uint16_t)(0x510 + track*2u));
        restart=jx_signed_word(elapsed)>=jx_signed_word(duration);
        if(!restart) {
            (void)JX_CALL(c,0x1832,0x2180,0x8738,c->data_segment);
            restart=jx_word(c,0x8738)!=0;
        }
    }
    if(restart) {
        uint16_t track=jx_word(c,0x889a);
        uint16_t resource=jx_far_word(c,jx_word(c,0x23ac),(uint16_t)(0x510 + track*2u));
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track)&255),2,resource);
    }
}
