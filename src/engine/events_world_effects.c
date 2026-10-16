#include "event_engine.h"
/* 2F28:9A20, inclusive palette range [first,last], three bytes per entry.
 * Signed validation precedes unsigned frame modulo. Palette output is the
 * complete 256-entry table, even when only a small range changed. */
void jx_palette_rotate_range(JxEventContext *c,uint16_t first,uint16_t last,
    uint16_t period) {
    if(jx_signed_word(period)<=0 || jx_signed_word(first)<0 || first>255 ||
       jx_signed_word(last)<0 || last>255 || last<=first)return;
    uint8_t saved[3];
    for(unsigned i=0;i<3;i++)saved[i]=jx_ds(c)[0x61de + 3u*first+i];
    if(jx_word(c,0x585e)%period!=0)return;
    for(uint16_t index=first;index<last;index++)
        for(unsigned i=0;i<3;i++)
            jx_ds(c)[0x61de + 3u*index+i]=jx_ds(c)[0x61de + 3u*(index+1)+i];
    for(unsigned i=0;i<3;i++)jx_ds(c)[0x61de + 3u*last+i]=saved[i];
    (void)JX_CALL(c,0x150f,0x0006,0x61de,c->data_segment,0,256);
}
/* 2F28:9B04. Only the entry gate is read once. Later range descriptors
 * remain live after each palette/device callback. */
void jx_world_palette_ranges(JxEventContext *c) {
    if(jx_signed_word(jx_word(c,0x6626))<=0)return;
    for(unsigned i=0;i<3;i++) {
        uint16_t at=(uint16_t)(0x6628+6*i);
        (void)JX_CALL(c,0x2f28,0x9a20,jx_word(c,at),
            jx_word(c,(uint16_t)(at+2)),jx_word(c,(uint16_t)(at+4)));
    }
}
/* 2F28:A1FA. An unconfigured/negative slot ends the entire update,
 * whereas a non-due sound proceeds to the next descriptor. */
void jx_world_periodic_sounds(JxEventContext *c) {
    for(unsigned i=0;i<3;i++) {
        uint16_t at=(uint16_t)(0x936c+6*i);
        uint16_t period=jx_word(c,(uint16_t)(at+2));
        if(period==0 || jx_signed_word(jx_word(c,at))<0)return;
        if(jx_word(c,0x585e)%period==0 &&
           (uint16_t)JX_CALL(c,0x1f2a,0x71de,jx_word(c,(uint16_t)(at+4)))==0)
            (void)JX_CALL(c,0x1f2a,0x155c,(uint16_t)(3+i));
    }
}
/* 2F28:A2E6. Every 1000 world frames (including frame zero), a nonzero
 * low AX from the disc check shows a message, waits 54 ticks and exits.
 * If the host's termination callback returns, the EXE also falls through. */
void jx_world_check_disc(JxEventContext *c) {
    if(jx_word(c,0x585e)%1000!=0)return;
    if((uint16_t)JX_CALL0(c,0x2f28,0x866a)==0)return;
    (void)JX_CALL(c,0x1f2a,0x0728,100,100,0x0e49,c->data_segment);
    (void)JX_CALL(c,0x1f2a,0x2dbc,54);
    (void)JX_CALL0(c,0x1f2a,0xb196);
}
