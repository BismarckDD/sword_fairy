#include "event_engine.h"
static uint16_t actor_menu_field(JxEventContext *c,uint16_t actor,uint16_t field) {
    return jx_word(c,(uint16_t)(0xd04e + actor*68u+field));
}
static void actor_menu_text(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer text) {
    (void)JX_CALL(c,0x1f2a,0x0728,x,y,text.offset,text.segment);
}
static void actor_menu_label(JxEventContext *c,uint16_t x,uint16_t y,uint16_t label) {
    actor_menu_text(c,x,y,(JxFarPointer){label,c->data_segment});
}
static void actor_menu_format(JxEventContext *c,JxFarPointer text,uint16_t format,uint16_t value) {
    (void)JX_CALL(c,0x1000,0x2a8c,text.offset,text.segment,format,c->data_segment,value);
}
static void actor_menu_append(JxEventContext *c,JxFarPointer text,JxFarPointer suffix) {
    (void)JX_CALL(c,0x1000,0x23f4,text.offset,text.segment,suffix.offset,suffix.segment);
}
static void actor_menu_ratio(JxEventContext *c,uint16_t actor,JxFarPointer text,JxFarPointer temp,uint16_t field,uint16_t maximum,uint16_t format,uint16_t separator,uint16_t max_format,uint16_t y) {
    actor_menu_format(c,text,format,actor_menu_field(c,actor,field));
    actor_menu_append(c,text,(JxFarPointer){separator,c->data_segment});
    actor_menu_format(c,temp,max_format,actor_menu_field(c,actor,maximum));
    actor_menu_append(c,text,temp);actor_menu_text(c,60,y,text);
}
/* 1F2A:864A.60-byte frame: temporary text at+2, main text at+26.
 * Level factor is captured before any EMS/UI call and reused for XP.
 * EMS failure exits immediately without cleanup, matching the original. */
void jx_actor_menu_show(JxEventContext *c,uint16_t actor,JxFarPointer work) {
    if(jx_signed_word(actor)<0 || jx_signed_word(actor)>4)return;
    uint16_t factor=(uint16_t)(jx_signed_word(actor_menu_field(c,actor,0x22))/10+1);
    for(uint16_t slot=0;slot<4;slot++)
        if((uint16_t)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(75+slot),jx_word(c,0x9332))!=0)return;
    (void)JX_CALL(c,0x1832,0x1810,2);
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    JxFarPointer text={(uint16_t)(work.offset+26),work.segment},temp={(uint16_t)(work.offset+2),work.segment};
    actor_menu_label(c,10,10,0x054c);
    actor_menu_ratio(c,actor,text,temp,0x10,0x14,0x0551,0x0555,0x0558,10);
    if(actor_menu_field(c,actor,0x24)==1)actor_menu_label(c,142,10,0x055c);
    else if(actor_menu_field(c,actor,0x24)==2)actor_menu_label(c,142,10,0x0561);
    actor_menu_label(c,10,30,0x0566);
    actor_menu_ratio(c,actor,text,temp,0x12,0x16,0x056b,0x056f,0x0572,30);
    uint16_t stat=actor_menu_field(c,actor,0x18);
    uint16_t item=actor_menu_field(c,actor,0x26);
    uint16_t amount=(uint16_t)(stat+jx_word(c,(uint16_t)(0x8022+item*28u+0x16)));
    actor_menu_format(c,text,0x0576,amount);actor_menu_label(c,10,50,0x057a);actor_menu_text(c,60,50,text);
    stat=actor_menu_field(c,actor,0x1a);item=actor_menu_field(c,actor,0x26);
    amount=(uint16_t)(stat+jx_word(c,(uint16_t)(0x8022+item*28u+0x18)));
    actor_menu_format(c,text,0x057f,amount);actor_menu_label(c,10,70,0x0583);actor_menu_text(c,60,70,text);
    actor_menu_format(c,text,0x0588,actor_menu_field(c,actor,0x22));actor_menu_label(c,10,90,0x058c);actor_menu_text(c,60,90,text);
    /* Original supplies32-bit money, but %5d consumes only its low word. */
    (void)JX_CALL(c,0x1000,0x2a8c,text.offset,text.segment,0x0591,c->data_segment,jx_word(c,0x8756),jx_word(c,0x8758));
    actor_menu_label(c,10,110,0x0595);actor_menu_text(c,60,110,text);
    actor_menu_label(c,10,130,0x059a);
    actor_menu_format(c,text,0x059f,actor_menu_field(c,actor,0x1e));
    actor_menu_append(c,text,(JxFarPointer){0x05a3,c->data_segment});
    uint16_t threshold=(uint16_t)((uint32_t)(uint16_t)(actor_menu_field(c,actor,0x22)*40u)*factor+50u);
    actor_menu_format(c,temp,0x05a6,threshold);actor_menu_append(c,text,temp);actor_menu_text(c,60,130,text);
    (void)JX_CALL(c,0x1f2a,0x752e,230,12,actor_menu_field(c,actor,0x0c));
    actor_menu_label(c,230,90,(uint16_t)(0xd04e + actor*68u));actor_menu_label(c,200,120,0x05aa);
    item=actor_menu_field(c,actor,0x26);actor_menu_label(c,250,120,(uint16_t)(0x8022+item*28u));
    uint32_t file=JX_CALL(c,0x1000,0x0738,0x05b4,c->data_segment,0x05b1,c->data_segment);
    uint32_t seek=(uint32_t)(jx_signed_word(actor_menu_field(c,actor,0x26))*3120);
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),(uint16_t)seek,(uint16_t)(seek>>16),0);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,3120,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1832,0x1a28,230,140,60,52,jx_word(c,0x2eb0),jx_word(c,0x2eb2),255);
    (void)JX_CALL(c,0x1832,0x1836,2);(void)JX_CALL0(c,0x1f2a,0x731a);
    if(jx_word(c,0x9362)!=0)(void)JX_CALL0(c,0x1f2a,0x65fc);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL0(c,0x1f2a,0xc24e);
}
