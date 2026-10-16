#include "event_engine.h"

/* 1F2A:B130. Release the live sound EMS handle, then print its message. */
void jx_shutdown_sound_storage(JxEventContext *c) {
    (void)JX_CALL(c,0x1503,0x009f,jx_word(c,0x9332));
    (void)JX_CALL(c,0x1000,0x0a80,0x0719,c->data_segment);
}
/* 1F2A:B156/B166/B182: preserve the original cleanup ordering. */
void jx_shutdown_keyboard_vector(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x6e8e);
}
void jx_shutdown_timer_callbacks(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0xa468,jx_word(c,0x6656));
    (void)JX_CALL(c,0x2f28,0xa3aa);
}
void jx_shutdown_text_mode(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0xa562,3);
}
/* 1F2A:B196. The exit service normally does not return. A host must stop
 * game execution there; returning is permitted only to inspect this sequence. */
void jx_game_shutdown(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x6d18,0x0744,c->data_segment);
    (void)JX_CALL(c,0x1f2a,0x7272);
    jx_shutdown_text_mode(c);
    jx_shutdown_timer_callbacks(c);
    jx_shutdown_keyboard_vector(c);
    jx_shutdown_sound_storage(c);
    if(jx_word(c,0x31b6)!=0)
        (void)JX_CALL(c,0x1f2a,0x2602,0x31b8,c->data_segment);
    if(jx_word(c,0x31b4)!=0)
        (void)JX_CALL(c,0x1832,0x22ce);
    (void)JX_CALL(c,0x1000,0x01eb,0);
}
