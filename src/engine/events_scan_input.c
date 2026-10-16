#include "event_engine.h"

/* Original1000:08C8 forwards its variadic argument area to1000:1036.
 * The %d and bounded %s branches used by the GO loader are implemented here.
 * Classification reads the original live CRT table; these scoped helpers
 * do not claim to implement the other scanf conversions. */
static int scan_space(JxEventContext *c,uint16_t value) {
    return (jx_ds(c)[(uint16_t)(0x15f7+value)]&8)!=0;
}
static uint16_t scan_first(JxEventContext *c,const JxScanInput *input) {
    uint16_t value;
    do {value=input->get(input->opaque);} while(scan_space(c,value));
    return value;
}
uint16_t jx_scan_decimal16(JxEventContext *c,const JxScanInput *input,uint16_t *output) {
    uint16_t ch=scan_first(c,input),value=0,digits=0;unsigned negative=0;
    if(ch=='-' || ch=='+'){negative=ch=='-';ch=input->get(input->opaque);}
    while(jx_ds(c)[(uint16_t)(0x15f7+ch)]&4){
        value=(uint16_t)(value*10+(uint16_t)(ch-'0'));++digits;
        ch=input->get(input->opaque);
    }
    if(ch!=65535)input->unget(input->opaque,ch);
    if(!digits)return ch==65535?65535:0;
    *output=negative?(uint16_t)(0-value):value;return 1;
}
uint16_t jx_scan_token(JxEventContext *c,const JxScanInput *input,uint8_t *output,uint16_t limit) {
    uint16_t ch=scan_first(c,input);unsigned count=0;
    while(ch!=65535 && !scan_space(c,ch) && count<limit){
        output[count++]=(uint8_t)ch;
        if(count==limit)break;
        ch=input->get(input->opaque);
    }
    if(count<limit && ch!=65535)input->unget(input->opaque,ch);
    if(!count)return ch==65535?65535:0;
    output[count]=0;return 1;
}
