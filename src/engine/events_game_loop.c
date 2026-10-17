#include "event_engine.h"
#include <string.h>
/* The caller still owns the original 142-byte frame. Labels use +2..81,
 * their far pointers +90..109 and selector input/CD scratch +110..141.
 * Each selector restores its background before entering another screen;
 * child screens cannot overwrite a parent's shared background/callbacks. */
static JxFarPointer loop_at(JxFarPointer frame,uint16_t offset) {
    return (JxFarPointer){(uint16_t)(frame.offset+offset),frame.segment};
}
static uint16_t loop_word(JxEventContext *c,JxFarPointer frame,uint16_t offset) {
    JxFarPointer p=loop_at(frame,offset);return jx_far_word(c,p.segment,p.offset);
}
static uint16_t choose(JxEventContext *c,JxFarPointer frame,uint16_t x,const char *const *names,uint16_t count,uint16_t *selected) {
    for(unsigned i=0;i<count;i++) {
        JxFarPointer text=loop_at(frame,(uint16_t)(2+16*i));size_t length=strlen(names[i]);
        for(unsigned k=0;k<16;k++)jx_far_set_byte(c,text,(uint16_t)k,k<length?(uint8_t)names[i][k]:0);
        JxFarPointer p=loop_at(frame,(uint16_t)(90+4*i));
        jx_far_set_byte(c,p,0,(uint8_t)text.offset);jx_far_set_byte(c,p,1,(uint8_t)(text.offset>>8));
        jx_far_set_byte(c,p,2,(uint8_t)text.segment);jx_far_set_byte(c,p,3,(uint8_t)(text.segment>>8));
    }
    return jx_menu_select(c,x,0,count,loop_at(frame,90),loop_at(frame,108),selected);
}
void jx_game_loop_initialize(JxEventContext *c,JxFarPointer frame) {
    (void)frame;(void)JX_CALL0(c,0x1f2a,0xc24e);
}
static int system_menu(JxEventContext *c,JxFarPointer frame) {
    uint16_t selected=0;
    for(;;) {
        int music=jx_word(c,0x31b4)!=0 && jx_word(c,0x050c)!=0;
        int sound=jx_word(c,0x31b6)!=0 && jx_word(c,0x050e)!=0;
        const char *names[]={music?"\xd2\xf4\xc0\xd6 \xbf\xaa":"\xd2\xf4\xc0\xd6 \xb9\xd8",
            sound?"\xd2\xf4\xd0\xa7 \xbf\xaa":"\xd2\xf4\xd0\xa7 \xb9\xd8",
            "\xb6\xc1\xb5\xb5","\xb4\xe6\xb5\xb5","\xcd\xcb\xb3\xf6\xd3\xce\xcf\xb7"};
        uint16_t action=choose(c,frame,76,names,5,&selected);
        if(action==0xffff)return 0;
        if(action==0)(void)JX_CALL0(c,0x1f2a,0x9d34);
        else if(action==1)(void)JX_CALL0(c,0x1f2a,0x9d6c);
        else if(action==2){if(jx_load_game_menu(c,(JxFarPointer){0,0x9d00})==0)return 1;}
        else if(action==3)jx_save_game_menu(c,(JxFarPointer){0,0x9d00});
        else {
            const char *exit_names[]={"\xcd\xcb\xb3\xf6\xd3\xce\xcf\xb7","\xc8\xa1\xcf\xfb"};uint16_t exit_selected=1;
            if(choose(c,frame,152,exit_names,2,&exit_selected)==0)(void)JX_CALL0(c,0x1f2a,0xb196);
        }
    }
}
static void main_menu(JxEventContext *c,JxFarPointer frame) {
    const char *names[]={"\xd7\xb4\xcc\xac","\xbc\xbc\xc4\xdc","\xce\xef\xc6\xb7","\xcf\xb5\xcd\xb3"};
    uint16_t selected=0;
    if(c->save_browser)(void)c->save_browser(c->save_browser_opaque,2);
    for(;;) {
        uint16_t action=choose(c,frame,0,names,4,&selected);
        if(action==0xffff)break;
        if(action==0)(void)JX_CALL0(c,0x1f2a,0xae12);
        else if(action==1)(void)JX_CALL0(c,0x1f2a,0xa892);
        else if(action==2)(void)JX_CALL0(c,0x1f2a,0xa8c0);
        else if(system_menu(c,frame))break;
    }
    (void)JX_CALL0(c,0x1f2a,0xc24e);
}
JxGameLoopStep jx_game_loop_step(JxEventContext *c,JxFarPointer frame) {
    JxFarPointer event=loop_at(frame,82);
    if((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event.offset,event.segment)!=0)return JX_GAME_LOOP_EMPTY;
    uint16_t type=loop_word(c,frame,82),key=loop_word(c,frame,84);
    if(type==8){(void)JX_CALL(c,0x2f28,0xa332);return JX_GAME_LOOP_PROCESSED;}
    if(type==0) {
        if(key==0x39 || key==0x1c)(void)JX_CALL0(c,0x1f2a,0x7606);
        else if(key==0x11)(void)JX_CALL0(c,0x1f2a,0x9d80);
    } else if(type==1 && key==1)main_menu(c,frame);
    return JX_GAME_LOOP_PROCESSED;
}
void jx_game_main_loop(JxEventContext *c,JxFarPointer frame) {
    jx_game_loop_initialize(c,frame);
    for(;;)(void)jx_game_loop_step(c,frame);
}
