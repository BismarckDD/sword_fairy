#include "event_engine.h"
/* 1F2A:72B2. Exit on event type0 or when signed timer count is strictly
 * greater than the signed duration. Counter wraps as a word: 32767 does
 * not expire on timer events alone. Other event types are ignored. */
void jx_wait_key_timed(JxEventContext *c,uint16_t duration,JxInputPoll poll,void *opaque) {
    uint16_t ticks=0;
    jx_input_clear(c);
    for(;;) {
        JxInputEvent event;
        if(poll(opaque,&event)!=0)continue;
        if(event.type==8) {
            ticks=(uint16_t)(ticks+1);
            if(jx_signed_word(ticks)<=jx_signed_word(duration))continue;
        } else if(event.type!=0)continue;
        jx_input_clear(c);return;
    }
}
