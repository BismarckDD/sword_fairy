#include "event_engine.h"

/* 1F2A:7272. Any key-down event ends the wait, with queue clearing at
 * entry and exit. Unlike 731A, this variant does not poll music. */
void jx_wait_key(JxEventContext *c,JxInputPoll poll,void *opaque) {
    jx_input_clear(c);
    for(;;) {
        JxInputEvent event;
        if(poll(opaque,&event)==0 && event.type==0) break;
    }
    jx_input_clear(c);
}

/* 1F2A:731A. This older music wait uses the LOCAL DS:0510 table and
 * optionally track 14. It is deliberately separate from 2F28:856C,
 * which uses a far resource table and a different restart policy. */
void jx_wait_key_music(JxEventContext *c,JxInputPoll poll,void *opaque) {
    uint16_t ticks=0;
    jx_input_clear(c);
    for(;;) {
        JxInputEvent event;
        if(poll(opaque,&event)!=0) continue;
        if(event.type==0) break;
        if(event.type!=8) continue;
        ticks++;
        if(jx_signed_word(jx_word(c,0x050c))<=0 || jx_signed_word(ticks)%3!=0) continue;
        uint16_t track=jx_word(c,0xacea)!=0?14:jx_word(c,0x889a);
        uint16_t table_slot=(uint16_t)(0x0510+2*track);
        int restart=(uint16_t)JX_CALL(c,0x1832,0x2212,0x933e,c->data_segment)!=0
            && jx_signed_word(track)>=0;
        if(!restart) {
            int elapsed=jx_ds(c)[0x9340]*60+jx_ds(c)[0x9341];
            restart=elapsed>=jx_signed_word(jx_word(c,table_slot)) && jx_signed_word(track)>=0;
        }
        if(!restart) {
            (void)JX_CALL(c,0x1832,0x2180,0x8738,c->data_segment);
            restart=jx_word(c,0x8738)!=0 && jx_signed_word(track)>=0;
        }
        if(restart) (void)JX_CALL(c,0x1832,0x224c,
            (uint16_t)((jx_ds(c)[0x932c]+track)&255),2,jx_word(c,table_slot));
    }
    jx_input_clear(c);
}
