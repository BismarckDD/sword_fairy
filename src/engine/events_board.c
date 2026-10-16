#include "event_engine.h"

static void board_sprite(JxEventContext *c,uint16_t column,uint16_t row,
    uint16_t resource,uint16_t mode) {
    (void)JX_CALL(c,0x1832,0x6e90,(uint16_t)(189+23u*column),
        (uint16_t)(3+23u*row),jx_word(c,resource),jx_word(c,(uint16_t)(resource+2)),mode);
}

/* Overlay 002:0398. Repaint occupied cells of the 19x19 board. */
void jx_board_draw(JxEventContext *c) {
    for(uint16_t row=0;row<19;row++) {
        for(uint16_t column=0;column<19;column++) {
            uint8_t piece=jx_ds(c)[0x5866+19u*row+column];
            if(piece==1) {
                board_sprite(c,column,row,0x873a,1);
                board_sprite(c,column,row,0x937e,0);
            } else if(piece==2) {
                board_sprite(c,column,row,0x3458,1);
                board_sprite(c,column,row,0x92b4,0);
            }
        }
    }
}

static uint16_t command_word(JxEventContext *c,JxFarPointer command,uint16_t offset) {
    return jx_far_word(c,command.segment,(uint16_t)(command.offset+offset));
}
static void command_sprite(JxEventContext *c,JxFarPointer command,uint16_t resource,uint16_t mode) {
    board_sprite(c,command_word(c,command,0),command_word(c,command,2),resource,mode);
}
static void command_store(JxEventContext *c,JxFarPointer command,uint8_t piece) {
    uint16_t row=command_word(c,command,2),column=command_word(c,command,0);
    jx_ds(c)[(uint16_t)(0x5866+row*19u+column)]=piece;
}

/* Overlay 002:0488. Six-byte records {column,row,operation}; operation 0
 * terminates, 1/2 place stones, 3 erases, other values are skipped.
 * Callback-visible commands are re-read after every call; the list pointer
 * advances with a wrapping offset and an unchanged segment. */
void jx_board_play_commands(JxEventContext *c,JxFarPointer command) {
    for(;;) {
        uint16_t operation=command_word(c,command,4);
        if(operation==0) {
            (void)JX_CALL0(c,0x1f2a,0x7272);
            return;
        }
        if(operation==1 || operation==2) {
            (void)JX_CALL(c,0x1f2a,0x155c,1);
            (void)JX_CALL(c,0x1503,0x007d,0,87,jx_word(c,0x9332));
            command_sprite(c,command,operation==1?0x873a:0x3458,1);
            command_sprite(c,command,operation==1?0x937e:0x92b4,0);
            command_store(c,command,(uint8_t)operation);
            (void)JX_CALL(c,0x1f2a,0x2dbc,18);
        } else if(operation==3) {
            command_sprite(c,command,0x5150,3);
            command_store(c,command,0);
        }
        command.offset=(uint16_t)(command.offset+6);
    }
}
