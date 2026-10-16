#include "event_engine.h"
/* 2F28:86CE. 2 means outside the map or occupied; 1 is terrain blocked.
 * EMS calls may change the live map pointer, width and handle. */
static JxWorldUpdateFlow movement_classify(JxEventContext *c,uint16_t x,uint16_t y,
        const JxFarPointer *scratch,uint16_t *result) {
    if(jx_signed_word(x)>=jx_signed_word(jx_word(c,0x663a)) ||
       jx_signed_word(y)>=jx_signed_word(jx_word(c,0x663c)) ||
       jx_signed_word(x)<0 || jx_signed_word(y)<2) {
        *result=2;return JX_WORLD_UPDATE_RETURNED;
    }
    if(scratch) {
        JxWorldUpdateFlow flow=jx_scene_cell_layers_world(c,(uint16_t)(x-jx_word(c,0x8896)),
            (uint16_t)(y-jx_word(c,0x8898)),*scratch);
        if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    } else (void)JX_CALL(c,0x2f28,0x492a,(uint16_t)(x-jx_word(c,0x8896)),(uint16_t)(y-jx_word(c,0x8898)));
    if(jx_signed_word(jx_word(c,0x9388))>=0) {*result=2;return JX_WORLD_UPDATE_RETURNED;}
    if(jx_word(c,0x050a)) {*result=0;return JX_WORLD_UPDATE_RETURNED;}
    for(uint16_t page=0;page<4;page++)
        (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(63+page),jx_word(c,0x9332));
    JxFarPointer tile={(uint16_t)(jx_word(c,0x5858)+2*x+2u*y*jx_word(c,0x663a)),jx_word(c,0x585a)};
    *result=jx_signed_word(jx_far_word(c,tile.segment,tile.offset))>=0?1:0;
    return JX_WORLD_UPDATE_RETURNED;
}
uint16_t jx_movement_classify(JxEventContext *c,uint16_t x,uint16_t y) {
    uint16_t result; (void)movement_classify(c,x,y,NULL,&result);return result;
}
JxWorldUpdateFlow jx_movement_classify_cells(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer scratch,uint16_t *result) {
    return movement_classify(c,x,y,&scratch,result);
}
/* 2F28:944C. Uppercase invokes a movement cycle; lowercase updates all
 * party facings. Unknown bytes still consume the requested delay. */
static JxCellFlow party_run_moves(JxEventContext *c,JxFarPointer commands,uint16_t delay,const JxFarPointer *scratch) {
    for(;;) {
        uint8_t command=jx_far_byte(c,commands,0);int direction=-1;
        if(!command) {
            jx_set_word(c,0x8784,2);jx_set_word(c,0x8786,2);jx_set_word(c,0x8788,2);
            if(scratch) return jx_party_redraw_trail_cells(c,*scratch);
            (void)JX_CALL(c,0x2f28,0x5166);return JX_CELL_RETURNED;
        }
        switch(command) {
        case 'U':case 'u':direction=0;break;
        case 'D':case 'd':direction=1;break;
        case 'L':case 'l':direction=2;break;
        case 'R':case 'r':direction=3;break;
        default:break;
        }
        if(direction>=0) {
            if(command<'a') {
                if(scratch) {
                    JxCellFlow flow=jx_party_move_cycle_cells(c,(uint16_t)direction,(uint16_t)direction,*scratch);
                    if(flow!=JX_CELL_RETURNED) return flow;
                } else (void)JX_CALL(c,0x2f28,0x8dd6,(uint16_t)direction,(uint16_t)direction);
            }
            else {
                jx_set_word(c,0x8764,(uint16_t)direction);jx_set_word(c,0x8770,(uint16_t)direction);jx_set_word(c,0x877c,(uint16_t)direction);
                if(scratch) {
                    jx_party_advance_frames(c);
                    JxCellFlow flow=jx_party_redraw_trail_cells(c,*scratch);
                    if(flow!=JX_CELL_RETURNED) return flow;
                } else {(void)JX_CALL(c,0x2f28,0x8c6e);(void)JX_CALL(c,0x2f28,0x5166);}
            }
        }
        commands.offset++;
        (void)JX_CALL(c,0x1f2a,0x2dbc,delay);
    }
}
/* 1F2A:CC24. Caller supplies the original private 256-byte path buffer.
 * Like the original, this requires a generated NUL-terminated path fitting
 * that buffer. Each choice reduces one coordinate difference; no BFS. */
void jx_party_walk_to(JxEventContext *c,uint16_t target_x,uint16_t target_y,JxFarPointer path) {
    jx_far_set_byte(c,path,0,0);
    uint16_t x=jx_word(c,0x8760),y=jx_word(c,0x8762);
    for(;;) {
        uint16_t text;
        switch(jx_choose_step(c,x,y,target_x,target_y)) {
        case 'U':y--;text=0x08b7;break;
        case 'D':y++;text=0x08b9;break;
        case 'L':x--;text=0x08bb;break;
        case 'R':x++;text=0x08bd;break;
        default:jx_party_run_moves(c,path,2);return;
        }
        (void)jx_string_append(c,path,(JxFarPointer){text,c->data_segment});
    }
}
/* 1F2A:CA66. Size is captured once; position and camera are live at each
 * tile callback. Signed, wrapped size+2 controls whether loops execute. */
static JxWorldUpdateFlow npc_redraw_background(JxEventContext *c,int16_t npc,const JxFarPointer *scratch) {
    if(npc<0) return JX_WORLD_UPDATE_RETURNED;
    uint16_t dimensions=(uint16_t)(0x31ce + (uint16_t)npc*18u);
    uint16_t width=jx_word(c,dimensions),height=jx_word(c,(uint16_t)(dimensions+2));
    uint16_t actor=(uint16_t)(0x889c + (uint16_t)npc*86u);
    int rows=jx_signed_word((uint16_t)(height+2)),columns=jx_signed_word((uint16_t)(width+2));
    for(int row=0;row<rows;row++) for(int column=0;column<columns;column++) {
        uint16_t y=(uint16_t)(jx_word(c,(uint16_t)(actor+0x42))-height+row-jx_word(c,0x8898));
        uint16_t x=(uint16_t)(jx_word(c,(uint16_t)(actor+0x40))-1+column-jx_word(c,0x8896));
        if(scratch) {
            JxWorldUpdateFlow flow=jx_scene_draw_tile_world(c,(int16_t)jx_signed_word(x),(int16_t)jx_signed_word(y),*scratch);
            if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
        } else (void)JX_CALL(c,0x2f28,0x4fdc,x,y);
    }
    return JX_WORLD_UPDATE_RETURNED;
}
void jx_npc_redraw_background(JxEventContext *c,int16_t npc) {
    (void)npc_redraw_background(c,npc,NULL);
}
JxWorldUpdateFlow jx_npc_redraw_background_world(JxEventContext *c,int16_t npc,JxFarPointer scratch) {
    return npc_redraw_background(c,npc,&scratch);
}
/* 1F2A:CD1C. Uppercase moves, lowercase only faces; all non-NUL bytes
 * consume one delay, including unknown commands. String offsets wrap. */
void jx_npc_run_moves(JxEventContext *c,uint16_t npc,JxFarPointer commands,uint16_t delay) {
    uint16_t actor=(uint16_t)(0x889c + npc*86u);
    for(;;) {
        uint8_t command=jx_far_byte(c,commands,0);int direction=-1;
        if(!command) {
            uint16_t kind=jx_word(c,(uint16_t)(0x31ce + npc*18u+4));
            jx_set_word(c,(uint16_t)(actor+0x3e),kind==5?2:0);
            jx_npc_redraw_background(c,(int16_t)jx_signed_word(npc));return;
        }
        switch(command) {
        case 'U':case 'u':direction=0;break;
        case 'D':case 'd':direction=1;break;
        case 'L':case 'l':direction=2;break;
        case 'R':case 'r':direction=3;break;
        default:break;
        }
        if(direction>=0) {
            if(command<'a') {
                uint16_t coordinate=(uint16_t)(actor+(direction<2?0x42:0x40));
                jx_set_word(c,coordinate,(uint16_t)(jx_word(c,coordinate)+((direction&1)?1:-1)));
            }
            jx_set_word(c,(uint16_t)(actor+0x44),(uint16_t)direction);
            jx_set_word(c,(uint16_t)(actor+0x3e),(uint16_t)(jx_word(c,(uint16_t)(actor+0x3e))+1));
            jx_npc_redraw_background(c,(int16_t)jx_signed_word(npc));
        }
        commands.offset++;
        (void)JX_CALL(c,0x1f2a,0x2dbc,delay);
    }
}
/* 1F2A:CB08, AL result. Greedy two-axis choice, not a pathfinder. Absolute
 * differences retain the original 8000 signed overflow before comparison. */
uint8_t jx_choose_step(JxEventContext *c,uint16_t x,uint16_t y,uint16_t target_x,uint16_t target_y) {
    int dx=jx_signed_word(jx_absolute_word((uint16_t)(x-target_x)));
    int dy=jx_signed_word(jx_absolute_word((uint16_t)(y-target_y)));
    int horizontal=dx>dy;
    for(unsigned attempt=0;attempt<2;attempt++,horizontal=!horizontal) {
        if(horizontal) {
            if(jx_signed_word(x)<jx_signed_word(target_x)) {
                if(!(uint16_t)JX_CALL(c,0x2f28,0x86ce,(uint16_t)(x+1),y)) return 'R';
            } else if(jx_signed_word(x)>jx_signed_word(target_x)) {
                if(!(uint16_t)JX_CALL(c,0x2f28,0x86ce,(uint16_t)(x-1),y)) return 'L';
            }
        } else {
            if(jx_signed_word(y)<jx_signed_word(target_y)) {
                if(!(uint16_t)JX_CALL(c,0x2f28,0x86ce,x,(uint16_t)(y+1))) return 'D';
            } else if(jx_signed_word(y)>jx_signed_word(target_y)) {
                if(!(uint16_t)JX_CALL(c,0x2f28,0x86ce,x,(uint16_t)(y-1))) return 'U';
            }
        }
    }
    return 0;
}

void jx_party_run_moves(JxEventContext *c,JxFarPointer commands,uint16_t delay) {(void)party_run_moves(c,commands,delay,NULL);}
JxCellFlow jx_party_run_moves_cells(JxEventContext *c,JxFarPointer commands,uint16_t delay,JxFarPointer scratch) {return party_run_moves(c,commands,delay,&scratch);}
