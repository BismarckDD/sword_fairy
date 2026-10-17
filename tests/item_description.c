#include "event_engine.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t memory[1048576];static JxEventContext context;
static unsigned drawn,clears,text_y=86;static char text[7][32];
static uint32_t dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    (void)opaque;
    if(segment==0x1832 && offset==0x1c6c && n==6 && a[0]==0 && a[1]==0 && a[2]==320 && a[3]==200)++clears;
    if(segment==0x1f2a && offset==0x0728 && n==4 && a[0]==8 && a[1]>=text_y) {
        assert(drawn<7 && a[1]==text_y+16*drawn);JxFarPointer p={a[2],a[3]};
        for(unsigned k=0;k<32;k++){text[drawn][k]=(char)jx_far_byte(&context,p,(uint16_t)k);if(!text[drawn][k])break;}++drawn;
    }
    return 0;
}
int main(void) {
    context=(JxEventContext){memory,0x39e3,dispatch,NULL,NULL,NULL};JxItemDescription d;
    const unsigned fields[]={0x0e,0x12,0x10,0x14,0x16,0x18};
    jx_set_word(&context,0xacea,1);
    for(unsigned i=0;i<6;i++)jx_set_word(&context,(uint16_t)(0x8022+42*28+fields[i]),(uint16_t)((i+1)*100));
    jx_item_description(&context,42,&d);assert(d.count==7);
    for(unsigned i=0;i<6;i++){char value[16];snprintf(value,sizeof value,"+%u",(i+1)*100);assert(strstr(d.lines[i],value));assert(strlen(d.lines[i])<30);}
    jx_set_word(&context,0x878a,42);jx_set_word(&context,0x878c,5);jx_set_word(&context,0xd05e,80);
    jx_item_menu_render(&context,0,0,0,(JxFarPointer){0,0x9c00});assert(clears==1 && drawn==7);
    for(unsigned i=0;i<7;i++)assert(strcmp(text[i],d.lines[i])==0);
    assert(jx_word(&context,0x878c)==5 && jx_word(&context,0xd05e)==80);
    drawn=0;text_y=132;jx_set_word(&context,0x878a,0);jx_item_menu_render(&context,0,0,0,(JxFarPointer){0,0x9c00});assert(clears==2 && drawn==1);
    jx_item_description(&context,12,&d);assert(d.count==3);
    jx_set_word(&context,0xacea,0);jx_item_description(&context,12,&d);assert(d.count==2);
    jx_set_word(&context,0x8022+1*28+0x16,30);jx_set_word(&context,0x8022+1*28+0x18,20);
    jx_item_description(&context,1,&d);assert(d.count==4 && strstr(d.lines[1],"+30") && strstr(d.lines[2],"+20"));
    jx_set_word(&context,0xacea,1);jx_set_word(&context,0x8022+1*28+0x0e,250);jx_item_description(&context,1,&d);assert(d.count==4 && strstr(d.lines[1],"+250"));
    jx_set_word(&context,0x8022+59*28+0x0e,50);jx_item_description(&context,59,&d);assert(d.count==4 && strstr(d.lines[1],"+50"));
    jx_item_description(&context,64,&d);assert(d.count==1);jx_item_description(&context,65535,&d);assert(d.count==1);
    puts("PASS: HP/MP restoration, maxima, attack/defense, weapons, projectiles, quest items, selection redraw/clear, seven-line bounds, and no gameplay state changes.");return 0;
}
