#include "event_engine.h"
static uint16_t item_menu_value(JxEventContext *c,uint16_t row,uint16_t column,uint16_t scroll) {
    return jx_word(c,(uint16_t)(0x878a+(((row+scroll)*2u+column)*4u)));
}
static uint16_t item_menu_page(uint16_t counter,unsigned advance) {
    return (uint16_t)(jx_signed_word((uint16_t)(counter+advance))%2+2);
}
static uint16_t item_menu_finish(JxEventContext *c,uint16_t value) {
    if(jx_word(c,0x9362)!=0)(void)JX_CALL0(c,0x1f2a,0x65fc);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL0(c,0x1f2a,0xc24e);return value;
}
/* 1F2A:9182. Two columns, ten visible rows, scroll0..22. Reserve36
 * frame bytes; event starts at+2. Empty item0 is returned on Enter.
 * At a viewport edge both row and scroll may change on the same key. */
uint16_t jx_item_menu(JxEventContext *c,JxFarPointer work) {
    uint16_t scroll=0,column=0,row=0,page_counter=2,ticks=0;
    (void)JX_CALL0(c,0x1f2a,0x8bd6);
    (void)JX_CALL(c,0x1832,0x1810,2);
    for(uint16_t slot=0;slot<4;slot++)(void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(79+slot),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+0x0c30;
    uint16_t backup_offset=(uint16_t)sum,backup_segment=(uint16_t)(jx_word(c,0x2eb2)+((sum>>16)<<12));
    (void)JX_CALL(c,0x1832,0x1e88,150,0,170,200,backup_offset,backup_segment);
    (void)JX_CALL(c,0x1f2a,0x8f78,column,row,scroll);
    (void)JX_CALL(c,0x1832,0x1836,2);(void)JX_CALL0(c,0x1f2a,0xc24e);
    JxFarPointer input={(uint16_t)(work.offset+2),work.segment};
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,input.offset,input.segment)!=0){}
        uint16_t type=jx_far_word(c,input.segment,input.offset);
        if(type==8) {
            ticks=(uint16_t)(ticks+1);
            if(jx_signed_word(ticks)%3==0 && jx_word(c,0x050c)==1 && jx_signed_word(jx_word(c,0x31b4))>0) {
                (void)JX_CALL(c,0x1832,0x2180,0x8738,c->data_segment);
                uint16_t track=jx_word(c,0xacea)!=0?14:jx_word(c,0x889a);
                if(jx_word(c,0x8738)!=0 && jx_signed_word(track)>=0) {
                    uint16_t duration=jx_word(c,(uint16_t)(0x0510+track*2u));
                    (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track)&255),2,duration);
                }
            }
        } else if(type==1) {
            uint16_t key=jx_far_word(c,input.segment,(uint16_t)(input.offset+2));
            if(key==1)return item_menu_finish(c,0xffff);
            if(key==0x48 || key==0x50 || key==0x4b || key==0x4d) {
                (void)JX_CALL(c,0x1f2a,0x155c,2);
                (void)JX_CALL(c,0x1832,0x135e,item_menu_page(page_counter,1),item_menu_page(page_counter,0));
                (void)JX_CALL(c,0x1832,0x1810,item_menu_page(page_counter,1));
                if(key==0x48) {
                    if(jx_signed_word(row)>0 && item_menu_value(c,(uint16_t)(row-1),column,scroll)!=0)row=(uint16_t)(row-1);
                    if(row==0 && jx_signed_word(scroll)>0 && item_menu_value(c,(uint16_t)(row-1),column,scroll)!=0)scroll=(uint16_t)(scroll-1);
                } else if(key==0x50) {
                    if(jx_signed_word(row)<9 && item_menu_value(c,(uint16_t)(row+1),column,scroll)!=0)row=(uint16_t)(row+1);
                    if(row==9 && jx_signed_word(scroll)<22 && item_menu_value(c,(uint16_t)(row+1),column,scroll)!=0)scroll=(uint16_t)(scroll+1);
                } else {
                    uint16_t candidate=column!=0?0:1;
                    if(item_menu_value(c,row,candidate,scroll)!=0)column=candidate;
                }
                (void)JX_CALL(c,0x1f2a,0x8f78,column,row,scroll);
                (void)JX_CALL(c,0x1832,0x1836,item_menu_page(page_counter,1));
                page_counter=(uint16_t)(page_counter+1);
            }
        }
        if(jx_far_word(c,input.segment,input.offset)==0 && (jx_far_word(c,input.segment,(uint16_t)(input.offset+2))==0x1c || jx_far_word(c,input.segment,(uint16_t)(input.offset+2))==0x39))
            return item_menu_finish(c,item_menu_value(c,row,column,scroll));
    }
}
