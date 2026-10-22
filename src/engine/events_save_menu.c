#include "event_engine.h"

static JxFarPointer save_work(JxFarPointer work,uint16_t off) {
    work.offset=(uint16_t)(work.offset+off);return work;
}
static JxFarPointer save_buffer(JxEventContext *c,uint16_t delta) {
    uint32_t off=(uint32_t)jx_word(c,0x2eb0)+delta;
    return (JxFarPointer){(uint16_t)off,(uint16_t)(jx_word(c,0x2eb2)+((off>>16)<<12))};
}
static void save_background(JxEventContext *c,uint16_t entry,JxFarPointer p) {
    (void)JX_CALL(c,0x1832,entry,116,40,72,100,p.offset,p.segment);
}
static void save_label(JxEventContext *c,uint16_t slot) {
    static const uint16_t names[]={0x60c,0x61a,0x628,0x636,0x644};
    (void)JX_CALL(c,0x1f2a,0x728,128,(uint16_t)(42+20*slot),names[slot],c->data_segment);
}
static void save_highlight(JxEventContext *c,uint16_t selected) {
    (void)JX_CALL(c,0x1f2a,0x6f2,47,0,77);
    save_label(c,selected);
    (void)JX_CALL(c,0x1f2a,0x6f2,80,0,77);
}
static void save_write(JxEventContext *c,uint32_t file,uint16_t dest,uint16_t size,uint16_t count) {
    (void)JX_CALL(c,0x1000,0x8e8,dest,c->data_segment,size,count,(uint16_t)file,(uint16_t)(file>>16));
}
/* 1F2A:9DC2. Five save slots; work reserves84 bytes. Escape restores
 * background and clears input, Enter writes the original17266-byte layout.
 * Open failure simply returns; individual write/close errors are ignored. */
void jx_save_game_menu(JxEventContext *c,JxFarPointer work) {
    if(c->save_browser && c->save_browser(c->save_browser_opaque,1)!=-2) {
        (void)JX_CALL0(c,0x1f2a,0xc24e);return;
    }
    uint16_t selection=0;
    save_background(c,0x1e88,save_buffer(c,0));
    JxFarPointer mapped=save_buffer(c,7200),source=save_buffer(c,0);
    (void)JX_CALL(c,0x2f28,0x6c7e,source.offset,source.segment,mapped.offset,mapped.segment,7200,0xb584,c->data_segment);
    save_background(c,0x1c6c,save_buffer(c,7200));
    for(uint16_t i=0;i<5;i++) {if(i==selection) save_highlight(c,i);else save_label(c,i);}
    (void)JX_CALL(c,0x2f28,0xa616,116,40,187,40,80);
    (void)JX_CALL(c,0x2f28,0xa616,116,139,187,139,80);
    (void)JX_CALL(c,0x2f28,0xa616,116,40,116,139,80);
    (void)JX_CALL(c,0x2f28,0xa616,187,40,187,139,80);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,work.offset,work.segment)!=0) {}
        uint16_t type=jx_far_word(c,work.segment,work.offset);
        uint16_t key=jx_far_word(c,work.segment,(uint16_t)(work.offset+2));
        if(type==8)save_highlight(c,selection);
        if(type==1) {
            if(key==1) {save_background(c,0x1c6c,save_buffer(c,0));(void)JX_CALL0(c,0x1f2a,0xc24e);return;}
            if(key==0x48 || key==0x50) {
                (void)JX_CALL(c,0x1f2a,0x155c,1);
                uint16_t previous=selection;
                selection=(uint16_t)((selection+(key==0x48?4:1))%5);
                save_highlight(c,selection);save_label(c,previous);
            }
        }
        type=jx_far_word(c,work.segment,work.offset);
        key=jx_far_word(c,work.segment,(uint16_t)(work.offset+2));
        if(type!=0 || (key!=0x1c && key!=0x39)) continue;
        save_background(c,0x1c6c,save_buffer(c,0));
        JxFarPointer filename=save_work(work,36),digits=save_work(work,72);
        (void)JX_CALL(c,0x1000,0x2448,filename.offset,filename.segment,0x64b,c->data_segment);
        (void)JX_CALL(c,0x1000,0x2a8c,digits.offset,digits.segment,0x655,c->data_segment,selection);
        (void)JX_CALL(c,0x1000,0x23f4,filename.offset,filename.segment,digits.offset,digits.segment);
        (void)JX_CALL(c,0x1000,0x23f4,filename.offset,filename.segment,0x650,c->data_segment);
        uint32_t file=JX_CALL(c,0x1000,0x738,filename.offset,filename.segment,0x659,c->data_segment);
        if(!file) return;
        save_write(c,file,0x8894,2,1);save_write(c,file,0x889a,2,1);
        save_write(c,file,0xd04e,68,16);save_write(c,file,0x8752,1,312);
        save_write(c,file,0x9334,1,2);save_write(c,file,0x3466,1,220);
        save_write(c,file,0x665a,1,6600);save_write(c,file,0xb686,1,6600);
        save_write(c,file,0xacec,1,2200);
        for(uint16_t i=0;i<30;i++) {
            uint16_t npc=(uint16_t)(0x889c+86*i);
            save_write(c,file,(uint16_t)(npc+64),1,6);
            save_write(c,file,(uint16_t)(npc+60),2,1);
        }
        (void)JX_CALL(c,0x1000,0x636,(uint16_t)file,(uint16_t)(file>>16));
        return;
    }
}
