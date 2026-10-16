#include "event_engine.h"

static uint16_t frame_page(JxEventContext *c,int next) {
    uint16_t frame=(uint16_t)(jx_word(c,0x3e54)+next);
    return (uint16_t)(jx_signed_word(frame)%2);
}
static void select_page(JxEventContext *c,uint16_t page) {
    (void)JX_CALL(c,0x1832,0x1810,page);
}
static void rectangle(JxEventContext *c,uint16_t operation,uint16_t x,uint16_t y,uint16_t width,uint16_t height) {
    (void)JX_CALL(c,0x1832,operation,x,y,width,height,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
}

/* 2F28:56EC: move the camera by tile units, animate the 16-pixel tile shift.
 * The low byte of direction is significant; no clamp is added to map limits.
 */
void jx_camera_scroll(JxEventContext *c,uint16_t direction,int16_t distance) {
    direction&=255;
    switch(direction) {
    case 'U':jx_set_word(c,0x8898,(uint16_t)(jx_word(c,0x8898)-distance));break;
    case 'D':jx_set_word(c,0x8898,(uint16_t)(jx_word(c,0x8898)+distance));break;
    case 'L':jx_set_word(c,0x8896,(uint16_t)(jx_word(c,0x8896)-distance));break;
    case 'R':jx_set_word(c,0x8896,(uint16_t)(jx_word(c,0x8896)+distance));break;
    default:return;
    }
    select_page(c,2);
    for(uint16_t row=0;row<12;++row)
        for(uint16_t column=0;column<20;++column)
            (void)JX_CALL(c,0x2f28,0x4fdc,column,row);
    select_page(c,frame_page(c,0));
    int pixels=jx_signed_word((uint16_t)((uint16_t)distance*16u));
    for(int step=0;step<pixels;++step) {
        if(direction=='U' || direction=='D') {
            (void)JX_CALL(c,0x1832,0x11a8,0,direction=='U' ? 0 : 1,320,191,0,
                         direction=='U' ? 1 : 0,frame_page(c,1),frame_page(c,0));
            select_page(c,2);
            uint16_t strip=(uint16_t)(direction=='U' ? pixels-step : 192-pixels+step);
            rectangle(c,0x1e88,0,strip,320,1);
            select_page(c,frame_page(c,1));
            rectangle(c,0x1c6c,0,direction=='U' ? 0 : 191,320,1);
        } else {
            rectangle(c,0x1e88,direction=='L' ? 0 : 1,0,319,192);
            select_page(c,frame_page(c,1));
            rectangle(c,0x1c6c,direction=='L' ? 1 : 0,0,319,192);
            select_page(c,2);
            uint16_t strip=(uint16_t)(direction=='L' ? pixels-step : 319-pixels+step);
            rectangle(c,0x1e88,strip,0,1,192);
            select_page(c,frame_page(c,1));
            rectangle(c,0x1c6c,direction=='L' ? 0 : 319,0,1,192);
        }
        (void)JX_CALL(c,0x1832,0x1836,frame_page(c,1));
        jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    }
}

/* 2F28:4572. Same modular-coordinate membership as the original nested loops.
 * Output pointers are untouched on failure, and may alias one another.
 */
int jx_rectangle_local_point(uint16_t *column,uint16_t *row,uint16_t x,uint16_t y,
                             uint16_t left,uint16_t top,int16_t width,int16_t height) {
    if(width<=0 || height<=0) return -1;
    uint16_t dx=(uint16_t)(x-left),dy=(uint16_t)(y-top);
    if(dx>=(uint16_t)width || dy>=(uint16_t)height) return -1;
    *column=dx;*row=dy;
    return 0;
}
