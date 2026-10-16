#include "event_engine.h"

static JxFarPointer load_work(JxFarPointer work,uint16_t off) {
    work.offset=(uint16_t)(work.offset+off);return work;
}
static JxFarPointer load_buffer(JxEventContext *c,uint16_t delta) {
    uint32_t off=(uint32_t)jx_word(c,0x2eb0)+delta;
    return (JxFarPointer){(uint16_t)off,(uint16_t)(jx_word(c,0x2eb2)+((off>>16)<<12))};
}
static void load_background(JxEventContext *c,uint16_t entry,JxFarPointer p) {
    (void)JX_CALL(c,0x1832,entry,116,40,72,100,p.offset,p.segment);
}
static void load_label(JxEventContext *c,uint16_t slot) {
    static const uint16_t names[]={0x663,0x671,0x67f,0x68d,0x69b};
    (void)JX_CALL(c,0x1f2a,0x728,128,(uint16_t)(42+20*slot),names[slot],c->data_segment);
}
static void load_highlight(JxEventContext *c,uint16_t selected) {
    (void)JX_CALL(c,0x1f2a,0x6f2,47,0,77);
    load_label(c,selected);
    (void)JX_CALL(c,0x1f2a,0x6f2,80,0,77);
}
static void load_read(JxEventContext *c,uint32_t file,uint16_t dest,uint16_t size,uint16_t count) {
    (void)JX_CALL(c,0x1000,0x754,dest,c->data_segment,size,count,(uint16_t)file,(uint16_t)(file>>16));
}
/* 1F2A:A2F0. Original menu has five slots, no existence preview, and only
 * type0 Enter confirms. work reserves 84 bytes matching the private frame;
 * the event, filename (offset36), and decimal scratch (offset72) are distinct.
 * File short reads/errors are intentionally not validated by this function. */
uint16_t jx_load_game_menu(JxEventContext *c,JxFarPointer work) {
    uint16_t selection=0;
    load_background(c,0x1e88,load_buffer(c,0));
    JxFarPointer mapped=load_buffer(c,7200),source=load_buffer(c,0);
    (void)JX_CALL(c,0x2f28,0x6c7e,source.offset,source.segment,mapped.offset,mapped.segment,7200,0xb584,c->data_segment);
    load_background(c,0x1c6c,load_buffer(c,7200));
    for(uint16_t i=0;i<5;i++) {if(i==selection) load_highlight(c,i);else load_label(c,i);}
    (void)JX_CALL(c,0x2f28,0xa616,116,40,187,40,80);
    (void)JX_CALL(c,0x2f28,0xa616,116,139,187,139,80);
    (void)JX_CALL(c,0x2f28,0xa616,116,40,116,139,80);
    (void)JX_CALL(c,0x2f28,0xa616,187,40,187,139,80);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,work.offset,work.segment)!=0) {}
        uint16_t type=jx_far_word(c,work.segment,work.offset);
        uint16_t key=jx_far_word(c,work.segment,(uint16_t)(work.offset+2));
        if(type==1) {
            if(key==1) {load_background(c,0x1c6c,load_buffer(c,0));return 1;}
            if(key==0x48 || key==0x50) {
                (void)JX_CALL(c,0x1f2a,0x155c,1);
                uint16_t previous=selection;
                selection=(uint16_t)((selection+(key==0x48?4:1))%5);
                load_highlight(c,selection);load_label(c,previous);
            }
        }
        type=jx_far_word(c,work.segment,work.offset);
        key=jx_far_word(c,work.segment,(uint16_t)(work.offset+2));
        if(type!=0 || key!=0x1c) continue;
        load_background(c,0x1c6c,load_buffer(c,0));
        (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
        JxFarPointer filename=load_work(work,36),digits=load_work(work,72);
        (void)JX_CALL(c,0x1000,0x2448,filename.offset,filename.segment,0x6a2,c->data_segment);
        (void)JX_CALL(c,0x1000,0x2a8c,digits.offset,digits.segment,0x6ac,c->data_segment,selection);
        (void)JX_CALL(c,0x1000,0x23f4,filename.offset,filename.segment,digits.offset,digits.segment);
        (void)JX_CALL(c,0x1000,0x23f4,filename.offset,filename.segment,0x6a7,c->data_segment);
        jx_set_word(c,0xb684,1);
        uint32_t file=JX_CALL(c,0x1000,0x738,filename.offset,filename.segment,0x6b0,c->data_segment);
        if(!file) (void)JX_CALL0(c,0x1f2a,0xb196); /* Original fatal shutdown. */
        load_read(c,file,0x8894,2,1);load_read(c,file,0x889a,2,1);
        load_read(c,file,0xd04e,68,16);load_read(c,file,0x8752,1,312);
        load_read(c,file,0x9334,1,2);load_read(c,file,0x3466,1,220);
        load_read(c,file,0x665a,1,6600);load_read(c,file,0xb686,1,6600);
        load_read(c,file,0xacec,1,2200);
        for(uint16_t i=0;i<30;i++) {
            uint16_t npc=(uint16_t)(0x889c+86*i);
            load_read(c,file,(uint16_t)(npc+64),1,6);
            load_read(c,file,(uint16_t)(npc+60),2,1);
        }
        (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
        (void)JX_CALL0(c,0x1f2a,0x55f2);
        if(jx_word(c,0x31b4) && jx_word(c,0x050c) && jx_signed_word(jx_word(c,0x889a))>=0) {
            uint16_t music=jx_word(c,0x889a);
            (void)JX_CALL(c,0x1832,0x224c,(uint16_t)(((jx_word(c,0x932c)&255)+music)&255),2,jx_word(c,(uint16_t)(0x510+2*music)));
        }
        (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
        jx_set_word(c,0xb684,0);return 0;
    }
}

