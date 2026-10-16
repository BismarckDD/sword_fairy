#include "event_engine.h"

static JxFarPointer board_pointer(JxEventContext *c,uint16_t slot) {
    return (JxFarPointer){jx_word(c,slot),jx_word(c,(uint16_t)(slot+2))};
}
static JxFarPointer board_huge_add(JxFarPointer p,unsigned amount) {
    unsigned sum=p.offset+amount;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(p.segment+(sum>>16)*4096u)};
}
static void board_word(JxEventContext *c,JxFarPointer p,uint16_t value) {
    uint32_t address=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[address]=(uint8_t)value;c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
}
static void board_cursor(JxEventContext *c,uint16_t column,uint16_t row) {
    (void)JX_CALL(c,0x1832,0x6e90,(uint16_t)(189+23*column),(uint16_t)(3+23*row),
        jx_word(c,0x888a),jx_word(c,0x888c),4);
}
static void board_read(JxEventContext *c,JxFarPointer file,JxFarPointer destination,uint16_t size) {
    (void)JX_CALL(c,0x1000,0x0754,destination.offset,destination.segment,1,size,file.offset,file.segment);
}

/* Overlay 002:0656. Set up the 640x480 planar board display, move a cursor
 * with scan codes and confirm a puzzle answer. Returns -1 for image open
 * failure, 0 for the correct square, 1 for a wrong square. No Escape branch
 * exists in the recovered routine. BIOS INT 1A's low timer word comes from
 * ticks(), called at the original point after the initial fade. */
int jx_board_run(JxEventContext *c,int16_t resource,JxInputPoll poll,JxTicksLow ticks,void *opaque) {
    uint16_t column=9,row=9;
    jx_event_transition_begin(c);
    (void)JX_CALL(c,0x1000,0x32a0,ticks(opaque));
    (void)JX_CALL(c,0x1503,0x007d,0,87,jx_word(c,0x9332));
    const uint16_t slots[]={0x937e,0x873a,0x92b4,0x3458,0x888a,0x5150};
    uint16_t previous=0x5858;
    for(unsigned i=0;i<6;i++) {
        jx_set_word(c,slots[i],(uint16_t)(jx_word(c,previous)+(i?516:0)));
        jx_set_word(c,(uint16_t)(slots[i]+2),jx_word(c,(uint16_t)(previous+2)));
        previous=slots[i];
    }
    board_word(c,board_pointer(c,0x2eb0),640);
    board_word(c,board_huge_add(board_pointer(c,0x2eb0),2),120);
    for(unsigned i=0;i<6;i++) {
        uint16_t size=i==5?22:32;
        board_word(c,board_pointer(c,slots[i]),size);
        JxFarPointer p=board_pointer(c,slots[i]);p.offset=(uint16_t)(p.offset+2);
        board_word(c,p,size);
    }
    uint32_t handle=JX_CALL(c,0x1000,0x0738,0x01ed,c->data_segment,0x01ea,c->data_segment);
    if(!handle) return -1;
    JxFarPointer file={(uint16_t)handle,(uint16_t)(handle>>16)};
    (void)JX_CALL(c,0x1000,0x27fa,file.offset,file.segment,0xffd0,0xffff,2);
    board_read(c,file,(JxFarPointer){0x65d4,c->data_segment},48);
    (void)jx_memory_fill(c,(JxFarPointer){0x4176,c->data_segment},0,200);
    /* The original ignores load failure and proceeds with remaining data. */
    (void)jx_board_load(c,resource);
    for(unsigned i=0;i<16;i++) {
        jx_ds(c)[0xacaa+i*4+2]=jx_ds(c)[0x65d4+i*3+2];
        jx_ds(c)[0xacaa+i*4+1]=jx_ds(c)[0x65d4+i*3+1];
        jx_ds(c)[0xacaa+i*4]=jx_ds(c)[0x65d4+i*3];
        jx_ds(c)[0xacaa+i*4+3]=0;
    }
    (void)JX_CALL(c,0x1832,0x69b3,18);
    (void)JX_CALL(c,0x1832,0x6ca3,0xacaa,c->data_segment);
    (void)JX_CALL(c,0x1000,0x27fa,file.offset,file.segment,16,0,0);
    for(uint16_t part=0;part<4;part++) {
        board_read(c,file,board_huge_add(board_pointer(c,0x2eb0),4),38400);
        (void)JX_CALL(c,0x1832,0x6e90,0,(uint16_t)(part*120),jx_word(c,0x2eb0),jx_word(c,0x2eb2),3);
    }
    (void)JX_CALL(c,0x1000,0x0636,file.offset,file.segment);
    const uint16_t names[]={0x01f7,0x0204,0x0212,0x021f,0x022d};
    const uint16_t modes[]={0x01f4,0x0201,0x020f,0x021c,0x022a};
    for(unsigned i=0;i<5;i++) {
        handle=JX_CALL(c,0x1000,0x0738,names[i],c->data_segment,modes[i],c->data_segment);
        if(!handle) return -1;
        file=(JxFarPointer){(uint16_t)handle,(uint16_t)(handle>>16)};
        (void)JX_CALL(c,0x1000,0x27fa,file.offset,file.segment,16,0,0);
        JxFarPointer destination=board_pointer(c,slots[i]);destination.offset=(uint16_t)(destination.offset+4);
        board_read(c,file,destination,512);
        (void)JX_CALL(c,0x1000,0x0636,file.offset,file.segment);
    }
    jx_board_draw(c);board_cursor(c,column,row);
    (void)JX_CALL(c,0x1832,0x6df7,212,26,234,48,jx_word(c,0x5150),jx_word(c,0x5152));
    jx_input_clear(c);
    for(;;) {
        JxInputEvent event;
        if(poll(opaque,&event)!=0 || event.type!=0) continue;
        int dx=0,dy=0;
        if(event.value1==0x48 && row>0) dy=-1;
        else if(event.value1==0x50 && row<18) dy=1;
        else if(event.value1==0x4b && column>0) dx=-1;
        else if(event.value1==0x4d && column<18) dx=1;
        if(dx || dy) {
            board_cursor(c,column,row);
            column=(uint16_t)(column+dx);row=(uint16_t)(row+dy);
            board_cursor(c,column,row);
        } else if(event.value1==0x39 || event.value1==0x1c) {
            (void)JX_CALL0(c,0x1f2a,0x5ae8);
            int correct=column==jx_word(c,0x3462) && row==jx_word(c,0x3464);
            if(correct) {
                board_cursor(c,column,row);
                jx_board_play_commands(c,(JxFarPointer){0x4176,c->data_segment});
            }
            (void)JX_CALL0(c,0x1832,0x17ae);
            (void)JX_CALL(c,0x150f,0x0006,0x61de,c->data_segment,0,256);
            jx_event_refresh(c);
            return correct?0:1;
        }
    }
}
