#include "event_engine.h"
static int skill_menu_entry(JxEventContext *c,uint16_t actor,uint16_t row,uint16_t column) {
    uint16_t at=(uint16_t)(0xd04e + actor*68u+0x34+row*2u+column);
    uint8_t value=jx_ds(c)[at];return value<128?value:(int)value-256;
}
static uint16_t skill_menu_page(uint16_t counter) {
    return (uint16_t)(jx_signed_word((uint16_t)(counter+1))%2+2);
}
static void skill_menu_draw(JxEventContext *c,uint16_t actor,uint16_t row,uint16_t column) {
    (void)JX_CALL(c,0x1f2a,0x9712,column,row,actor);
    (void)JX_CALL(c,0x1f2a,0x752e,20,10,jx_word(c,(uint16_t)(0xd04e + actor*68u+0x0c)));
}
/* 1F2A:9952. Eight rows with two signed-byte skill slots each.
 * Navigation tests only the adjacent slot; it does not skip empty slots.
 * Input owns eight bytes at work+2 within the original34-byte frame. */
uint16_t jx_skill_menu(JxEventContext *c,uint16_t actor,JxFarPointer work) {
    uint16_t page_counter=2,column=0,row=0,ticks=0,result;
    if(jx_signed_word(actor)<0 || jx_signed_word(actor)>4)return 0xffff;
    (void)JX_CALL(c,0x1832,0x1810,2);skill_menu_draw(c,actor,row,column);
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
        }
        if(jx_far_word(c,input.segment,input.offset)==1) {
            uint16_t key=jx_far_word(c,input.segment,(uint16_t)(input.offset+2));
            if(key==1){result=0xffff;break;}
            if(key==0x48 || key==0x50 || key==0x4b || key==0x4d) {
                (void)JX_CALL(c,0x1f2a,0x155c,0);
                (void)JX_CALL(c,0x1832,0x1810,skill_menu_page(page_counter));
                if(key==0x48 || key==0x50) {
                    uint16_t candidate=(uint16_t)(jx_signed_word((uint16_t)(row+(key==0x48?7:1)))%8);
                    if(skill_menu_entry(c,actor,candidate,column)>=0)row=candidate;
                } else {
                    uint16_t candidate=column!=0?0:1;
                    if(skill_menu_entry(c,actor,row,candidate)>=0)column=candidate;
                }
                skill_menu_draw(c,actor,row,column);
                (void)JX_CALL(c,0x1832,0x1836,skill_menu_page(page_counter));
                page_counter=(uint16_t)(page_counter+1);
            }
        }
        if(jx_far_word(c,input.segment,input.offset)==0 && jx_far_word(c,input.segment,(uint16_t)(input.offset+2))==0x1c) {
            result=(uint16_t)skill_menu_entry(c,actor,row,column);break;
        }
    }
    if(jx_word(c,0x9362)!=0)(void)JX_CALL0(c,0x1f2a,0x65fc);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL0(c,0x1f2a,0xc24e);return result;
}
