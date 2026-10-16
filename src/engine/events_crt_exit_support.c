#include "event_engine.h"

/* 1000:028B near, SI/DI range. Entries are visited backwards and fetched
 * anew after each callback. Empty Far entries are skipped. */
void jx_crt_run_exit_callbacks(JxEventContext *c,uint16_t lower,uint16_t upper) {
    while(lower<upper) {
        upper=(uint16_t)(upper-4);
        uint16_t offset=jx_far_word(c,c->data_segment,upper);
        uint16_t segment=jx_far_word(c,c->data_segment,(uint16_t)(upper+2));
        if((offset|segment)!=0)(void)JX_CALL0(c,segment,offset);
    }
}
/* 1000:0556. The FFFF sentinel is tested after equality, so requesting
 * code FFFF returns the byte following its two-byte identifier. */
uint16_t jx_crt_error_message(JxEventContext *c,uint16_t code) {
    uint16_t cursor=0x23d3;
    for(;;) {
        uint16_t identifier=jx_far_word(c,c->data_segment,cursor);
        cursor=(uint16_t)(cursor+2);
        if(identifier==code)return cursor;
        if(identifier==0xffff)return 0;
        uint16_t count=0xffff;
        do {
            uint8_t value=jx_ds(c)[cursor];cursor=(uint16_t)(cursor+1);
            count=(uint16_t)(count-1);
            if(value==0)break;
        } while(count!=0);
    }
}
/* 1000:029E. Both messages and the optional live Far callback are ordered. */
void jx_crt_report_corruption(JxEventContext *c) {
    (void)JX_CALL(c,0x1000,0x0581,0xfc);
    uint16_t segment=jx_word(c,0x134c);
    if(segment!=0)(void)JX_CALL0(c,segment,jx_word(c,0x134a));
    (void)JX_CALL(c,0x1000,0x0581,0xff);
}
/* 1000:02EA. A successful checksum returns the final input byte in AL,
 * rather than a fabricated Boolean zero. */
uint16_t jx_crt_check_data(JxEventContext *c) {
    uint8_t checksum=0,last=0;
    for(uint16_t i=0;i<0x42;i++) {last=jx_ds(c)[i];checksum^=last;}
    checksum^=0x55;
    if(checksum==0)return last;
    jx_crt_report_corruption(c);
    (void)JX_CALL(c,0x1000,0x0581,1);
    return 1;
}
