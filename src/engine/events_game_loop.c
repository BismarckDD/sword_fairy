#include "event_engine.h"

/* 1F2A:AE40. Caller-owned 142-byte frame retains the original local layout:
 * +2 five 16-byte option labels, +82 event, +90 five far pointers,
 * +110 loop counter, +112 three alternate-menu far pointers.
 * Init/step are host scheduling boundaries, not additional original entries. */
static JxFarPointer loop_at(JxFarPointer frame,uint16_t offset) {
    return (JxFarPointer){(uint16_t)(frame.offset+offset),frame.segment};
}
static uint16_t loop_word(JxEventContext *c,JxFarPointer frame,uint16_t offset) {
    JxFarPointer p=loop_at(frame,offset);return jx_far_word(c,p.segment,p.offset);
}
static void loop_set(JxEventContext *c,JxFarPointer frame,uint16_t offset,uint16_t value) {
    JxFarPointer p=loop_at(frame,offset);uint32_t at=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
void jx_game_loop_initialize(JxEventContext *c,JxFarPointer frame) {
    const uint16_t labels[5]={0x06b9,0x06c5,0x06d1,0x06dd,0x06e9};
    for(unsigned i=0;i<5;i++) {
        JxFarPointer text=loop_at(frame,(uint16_t)(2+16*i));
        (void)JX_CALL(c,0x1000,0x3514,text.offset,text.segment,labels[i],c->data_segment,6);
        JxFarPointer tail=loop_at(text,6);
        (void)JX_CALL(c,0x1000,0x3572,tail.offset,tail.segment,0,10);
    }
    const uint16_t submenu[3]={0x06f6,0x0704,0x0712};
    for(unsigned i=0;i<3;i++) {
        loop_set(c,frame,(uint16_t)(112+4*i),submenu[i]);
        loop_set(c,frame,(uint16_t)(114+4*i),c->data_segment);
    }
    loop_set(c,frame,110,0);
    while(jx_signed_word(loop_word(c,frame,110))<5) {
        uint16_t index=loop_word(c,frame,110);
        jx_far_set_byte(c,frame,(uint16_t)(10+16*index),0);
        loop_set(c,frame,110,(uint16_t)(loop_word(c,frame,110)+1));
    }
    (void)JX_CALL(c,0x1f2a,0xc24e);
}
static void menu_callbacks(JxEventContext *c,const uint16_t *entries,unsigned count) {
    for(unsigned i=0;i<count;i++) {
        jx_set_word(c,(uint16_t)(0x8722+4*i),entries[i]);
        jx_set_word(c,(uint16_t)(0x8724+4*i),0x1f2a);
    }
}
JxGameLoopStep jx_game_loop_step(JxEventContext *c,JxFarPointer frame) {
    JxFarPointer event=loop_at(frame,82);
    if((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event.offset,event.segment)!=0)
        return JX_GAME_LOOP_EMPTY;
    uint16_t type=loop_word(c,frame,82);
    if(type==8) {
        (void)JX_CALL(c,0x2f28,0xa332);return JX_GAME_LOOP_PROCESSED;
    }
    if(type==0) {
        uint16_t key=loop_word(c,frame,84);
        if(key==0x39)(void)JX_CALL(c,0x1f2a,0x7606);
        else if(key==0x11)(void)JX_CALL(c,0x1f2a,0x9d80);
        else if(key==0x1c) {
            const uint16_t callbacks[3]={0xae12,0xa8c0,0xa892};
            menu_callbacks(c,callbacks,3);
            JxFarPointer table=loop_at(frame,112);
            (void)JX_CALL(c,0x1f2a,0x80fa,248,0,3,table.offset,table.segment);
        }
        return JX_GAME_LOOP_PROCESSED;
    }
    if(type!=1 || loop_word(c,frame,84)!=1)return JX_GAME_LOOP_PROCESSED;
    const uint16_t callbacks[5]={0x9d34,0x9d6c,0xa2f0,0x9dc2,0xb196};
    menu_callbacks(c,callbacks,5);
    jx_far_set_byte(c,frame,7,32);jx_far_set_byte(c,frame,23,32);
    int music=jx_word(c,0x31b4)!=0 && jx_word(c,0x050c)!=0;
    jx_far_set_byte(c,frame,8,music?0xbf:0xb9);
    jx_far_set_byte(c,frame,9,music?0xaa:0xd8);
    int sound=jx_word(c,0x31b6)!=0 && jx_word(c,0x050e)!=0;
    jx_far_set_byte(c,frame,24,sound?0xbf:0xb9);
    jx_far_set_byte(c,frame,25,sound?0xaa:0xd8);
    loop_set(c,frame,110,0);
    while(jx_signed_word(loop_word(c,frame,110))<5) {
        uint16_t index=loop_word(c,frame,110);JxFarPointer text=loop_at(frame,(uint16_t)(2+16*index));
        loop_set(c,frame,(uint16_t)(90+4*index),text.offset);
        loop_set(c,frame,(uint16_t)(92+4*index),text.segment);
        loop_set(c,frame,110,(uint16_t)(loop_word(c,frame,110)+1));
    }
    JxFarPointer table=loop_at(frame,90);
    (void)JX_CALL(c,0x1f2a,0x80fa,0,0,5,table.offset,table.segment);
    return JX_GAME_LOOP_PROCESSED;
}
void jx_game_main_loop(JxEventContext *c,JxFarPointer frame) {
    jx_game_loop_initialize(c,frame);
    for(;;)(void)jx_game_loop_step(c,frame);
}
