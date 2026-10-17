#include "event_engine.h"
#include <stdio.h>
#include <string.h>
/* Descriptions follow 3F12/A8C0's actual field mapping, not item names.
 * 0E/12 restore HP/MP, 10/14 increase their maxima, 16/18 add ATK/DEF. */
static void description_line(JxItemDescription *d,const char *text) {
    if(d->count<7)snprintf(d->lines[d->count++],sizeof d->lines[0],"%s",text);
}
static void description_amount(JxItemDescription *d,const char *label,unsigned value) {
    if(value && d->count<7)snprintf(d->lines[d->count++],sizeof d->lines[0],"%s+%u",label,value);
}
void jx_item_description(JxEventContext *c,uint16_t item,JxItemDescription *d) {
    memset(d,0,sizeof *d);
    if(!item || item>=64){description_line(d,"\xc7\xeb\xd1\xa1\xd4\xf1\xce\xef\xc6\xb7");return;}
    int battle=jx_word(c,0xacea)!=0;uint16_t base=(uint16_t)(0x8022+28*item);
    if(item>=38 && item<=58) {
        static const uint16_t fields[]={0x0e,0x12,0x10,0x14,0x16,0x18};
        static const char *labels[]={"\xbb\xd6\xb8\xb4\xc9\xfa\xc3\xfc","\xbb\xd6\xb8\xb4\xc4\xda\xc1\xa6","\xc9\xfa\xc3\xfc\xc9\xcf\xcf\xde","\xc4\xda\xc1\xa6\xc9\xcf\xcf\xde","\xb9\xa5\xbb\xf7\xcc\xe1\xc9\xfd","\xb7\xc0\xd3\xf9\xcc\xe1\xc9\xfd"};
        for(unsigned i=0;i<6;i++)description_amount(d,labels[i],jx_word(c,(uint16_t)(base+fields[i])));
        if(!d->count)description_line(d,"\xce\xde\xbb\xd6\xb8\xb4\xd0\xa7\xb9\xfb");
        if(d->count<6 && (jx_word(c,(uint16_t)(base+0x0e)) || jx_word(c,(uint16_t)(base+0x12))))description_line(d,"\xbb\xd6\xb8\xb4\xb2\xbb\xb3\xac\xc9\xcf\xcf\xde");
        description_line(d,battle?"\xd5\xbd\xb6\xb7\xd6\xd0\xd3\xc3\xd3\xda\xd7\xd4\xc9\xed":"\xca\xb9\xd3\xc3\xca\xb1\xd1\xa1\xd4\xf1\xbd\xc7\xc9\xab");
    } else if(item<12 && !battle) {
        description_line(d,"\xce\xe4\xc6\xf7\xd7\xb0\xb1\xb8");description_amount(d,"\xb9\xa5\xbb\xf7",jx_word(c,(uint16_t)(base+0x16)));
        description_amount(d,"\xb7\xc0\xd3\xf9",jx_word(c,(uint16_t)(base+0x18)));
        description_line(d,"\xca\xb9\xd3\xc3\xca\xb1\xd1\xa1\xd4\xf1\xbd\xc7\xc9\xab");
    } else if(item<12 || item>=59) {
        description_line(d,item<12?"\xcd\xb6\xd6\xc0\xce\xe4\xc6\xf7":"\xb0\xb5\xc6\xf7");
        description_amount(d,"\xc9\xcb\xba\xa6\xbb\xf9\xca\xfd",jx_word(c,(uint16_t)(base+0x0e)));
        description_line(d,"\xd5\xbd\xb6\xb7\xd6\xd0\xb9\xa5\xbb\xf7\xb5\xd0\xc8\xcb");description_line(d,"\xca\xb9\xd3\xc3\xba\xf3\xcf\xfb\xba\xc4");
    } else {
        description_line(d,"\xbe\xe7\xc7\xe9\xce\xef\xc6\xb7");description_line(d,"\xd3\xc3\xd3\xda\xb5\xf7\xb2\xe9\xbb\xf2\xbd\xbb\xb8\xb6");
        if(battle)description_line(d,"\xd5\xbd\xb6\xb7\xd6\xd0\xce\xde\xb7\xa8\xca\xb9\xd3\xc3");
    }
}
