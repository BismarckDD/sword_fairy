#include "event_engine.h"
static JxFarPointer menu_buffer(JxEventContext *c,int displaced,uint16_t count) {
    uint16_t offset=jx_word(c,0x2eb0),segment=jx_word(c,0x2eb2);
    if(displaced) {
        uint16_t amount=(uint16_t)(count*1440u);
        uint32_t sum=(uint32_t)offset+amount;
        int high=(amount>=32768?-1:0)+(int)(sum>>16);
        offset=(uint16_t)sum;segment=(uint16_t)(segment+high*4096);
    }
    JxFarPointer p={offset,segment};return p;
}
static void menu_restore(JxEventContext *c,uint16_t x,uint16_t y,uint16_t count) {
    JxFarPointer p=menu_buffer(c,0,count);
    (void)JX_CALL(c,0x1832,0x1c6c,x,y,72,(uint16_t)(count*20u),p.offset,p.segment);
}
static void menu_reset_callbacks(JxEventContext *c) {
    for(unsigned i=0;i<5;i++) {
        jx_set_word(c,(uint16_t)(0x8722+i*4),0x86ba);
        jx_set_word(c,(uint16_t)(0x8724+i*4),0x2f28);
    }
}
static void menu_text(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer labels,uint16_t row) {
    uint16_t p=(uint16_t)(labels.offset+row*4u);
    uint16_t offset=jx_far_word(c,labels.segment,p);
    uint16_t segment=jx_far_word(c,labels.segment,(uint16_t)(p+2));
    (void)JX_CALL(c,0x1f2a,0x0728,(uint16_t)(x+4),(uint16_t)(y+row*20u+1),offset,segment);
}
static void menu_cd_play(JxEventContext *c,uint16_t track) {
    uint16_t duration=jx_word(c,(uint16_t)(0x0510+track*2u));
    (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track)&255),2,duration);
}
/* 1F2A:80FA. work corresponds to the34-byte BP frame. Input writes
 * start at+2, CD status at+20. Input polling intentionally has no timeout. */
static uint16_t action_menu_run(JxEventContext *c,uint16_t x,uint16_t y,uint16_t count,JxFarPointer labels,JxFarPointer work,uint16_t *selection,int callbacks) {
    if(!count || count>5)return 0xffff;
    uint16_t selected=*selection<count?*selection:0,ticks=0;
    JxFarPointer p=menu_buffer(c,0,count);
    (void)JX_CALL(c,0x1832,0x1e88,x,y,72,(uint16_t)(count*20u),p.offset,p.segment);
    JxFarPointer src=menu_buffer(c,1,count);p=menu_buffer(c,0,count);
    (void)JX_CALL(c,0x2f28,0x6c7e,p.offset,p.segment,src.offset,src.segment,(uint16_t)(count*1440u),0xb584,c->data_segment);
    p=menu_buffer(c,1,count);
    (void)JX_CALL(c,0x1832,0x1c6c,x,y,72,(uint16_t)(count*20u),p.offset,p.segment);
    uint16_t right=(uint16_t)(x+71),bottom=(uint16_t)(y+count*20u-1);
    (void)JX_CALL(c,0x2f28,0xa616,x,y,right,y,80);
    (void)JX_CALL(c,0x2f28,0xa616,x,bottom,right,bottom,80);
    (void)JX_CALL(c,0x2f28,0xa616,x,y,x,bottom,80);
    (void)JX_CALL(c,0x2f28,0xa616,right,y,right,bottom,80);
    for(uint16_t row=0;jx_signed_word(row)<jx_signed_word(count);row++) {
        if(row==selected)(void)JX_CALL(c,0x1f2a,0x06f2,47,0,77);
        menu_text(c,x,y,labels,row);
        if(row==selected)(void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
    }
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        JxFarPointer input={(uint16_t)(work.offset+2),work.segment};
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,input.offset,input.segment)!=0){}
        uint16_t type=jx_far_word(c,input.segment,input.offset);
        if(type==8) {
            (void)JX_CALL(c,0x1f2a,0x06f2,47,0,77);
            menu_text(c,x,y,labels,selected);
            (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
            ticks=(uint16_t)(ticks+1);
            if(jx_signed_word(jx_word(c,0x050c))>0 && jx_signed_word(ticks)%3==0) {
                uint16_t track=jx_word(c,0xacea)!=0?14:jx_word(c,0x889a);
                JxFarPointer status={(uint16_t)(work.offset+20),work.segment};
                uint16_t ready=(uint16_t)JX_CALL(c,0x1832,0x2212,status.offset,status.segment);
                if(ready!=0 && jx_signed_word(track)>=0){menu_cd_play(c,track);continue;}
                uint16_t elapsed=(uint16_t)(jx_far_byte(c,status,2)*60u+jx_far_byte(c,status,3));
                if(jx_signed_word(elapsed)>=jx_signed_word(jx_word(c,(uint16_t)(0x0510+track*2u))) && jx_signed_word(track)>=0){menu_cd_play(c,track);continue;}
                (void)JX_CALL(c,0x1832,0x2180,0x8738,c->data_segment);
                if(jx_word(c,0x8738)!=0 && jx_signed_word(track)>=0)menu_cd_play(c,track);
            }
        } else if(type==1) {
            uint16_t key=jx_far_word(c,input.segment,(uint16_t)(input.offset+2));
            if(key==1){menu_restore(c,x,y,count);if(callbacks)menu_reset_callbacks(c);*selection=selected;return 0xffff;}
            if(key==0x48 || key==0x50) {
                (void)JX_CALL(c,0x1f2a,0x155c,1);
                uint16_t previous=selected;
                uint16_t numerator=key==0x48?(uint16_t)(selected+count-1):(uint16_t)(selected+1);
                /* Original idiv requires nonzero count when navigating. */
                selected=(uint16_t)(jx_signed_word(numerator)%jx_signed_word(count));
                (void)JX_CALL(c,0x1f2a,0x06f2,47,0,77);menu_text(c,x,y,labels,selected);
                (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);menu_text(c,x,y,labels,previous);
            }
        }
        if(jx_far_word(c,input.segment,input.offset)==0 && (jx_far_word(c,input.segment,(uint16_t)(input.offset+2))==0x1c || jx_far_word(c,input.segment,(uint16_t)(input.offset+2))==0x39)) {
            menu_restore(c,x,y,count);
            *selection=selected;
            if(!callbacks)return selected;
            uint16_t table=(uint16_t)(0x8722+selected*4u);
            uint16_t offset=jx_word(c,table),segment=jx_word(c,(uint16_t)(table+2));
            (void)JX_CALL0(c,segment,offset);
            menu_reset_callbacks(c);return 0;
        }
    }
}

uint16_t jx_action_menu(JxEventContext *c,uint16_t x,uint16_t y,uint16_t count,JxFarPointer labels,JxFarPointer work) {
    uint16_t selection=0;return action_menu_run(c,x,y,count,labels,work,&selection,1);
}
uint16_t jx_menu_select(JxEventContext *c,uint16_t x,uint16_t y,uint16_t count,JxFarPointer labels,JxFarPointer work,uint16_t *selection) {
    return action_menu_run(c,x,y,count,labels,work,selection,0);
}
