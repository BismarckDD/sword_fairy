/* Shared engine bodies reconstructed from resident code. Lower drawing and
 * timing services remain explicit dependencies in the engine-call interface.
 */
#include "event_engine.h"

/* 2F28:56CC / 56DC: inhibit or enable automatic camera recentering. */
void jx_camera_lock(JxEventContext *c,int16_t unused) {
    (void)unused;jx_set_word(c,0x0ddc,1);
}
void jx_camera_unlock(JxEventContext *c,int16_t unused) {
    (void)unused;jx_set_word(c,0x0ddc,0);
}

/* 2F28:5580. Preserve signed 16-bit comparisons, including wrapped map limits. */
static JxWorldUpdateFlow scene_redraw(JxEventContext *c,const JxFarPointer *scratch) {
    if(jx_word(c,0x0ddc)==0) {
        uint16_t x=jx_word(c,0x8760),y=jx_word(c,0x8762);
        uint16_t width=jx_word(c,0x663a),height=jx_word(c,0x663c);
        uint16_t left=0,top=0;
        if(jx_signed_word(x)>9) {
            left=jx_signed_word(x)>=jx_signed_word((uint16_t)(width-11)) ? (uint16_t)(width-20) : (uint16_t)(x-9);
        }
        jx_set_word(c,0x8896,left);
        if(jx_signed_word(y)>6) {
            top=jx_signed_word(y)>=jx_signed_word((uint16_t)(height-6)) ? (uint16_t)(height-12) : (uint16_t)(y-6);
        }
        jx_set_word(c,0x8898,top);
    }
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    for(uint16_t row=0;row<12;++row) {
        for(uint16_t column=0;column<20;++column) {
            if(scratch) {
                JxWorldUpdateFlow flow=jx_scene_draw_tile_world(c,(int16_t)column,(int16_t)row,*scratch);
                if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
            } else (void)JX_CALL(c,0x2f28,0x4fdc,column,row);
        }
    }
    if(jx_word(c,0x08c8)==1) {
        for(uint16_t page=0;page<4;++page) {
            (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(0x75 + page),jx_word(c,0x9332));
        }
        (void)JX_CALL(c,0x1832,0x1a28,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a),255);
    }
    (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    return JX_WORLD_UPDATE_RETURNED;
}
void jx_scene_redraw(JxEventContext *c,int16_t unused) {
    (void)unused;(void)scene_redraw(c,NULL);
}
JxCellFlow jx_scene_redraw_cells(JxEventContext *c,JxFarPointer scratch) {
    return scene_redraw(c,&scratch)==JX_WORLD_UPDATE_RETURNED?JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;
}

/* 1F2A:DB46: advance NPC animation frames; first/last frames are not rendered. */
void jx_npc_play_animation(JxEventContext *c,int16_t npc) {
    int count=jx_signed_word(jx_word(c,(uint16_t)(0x31ce + (uint16_t)npc*18u+6)));
    for(int frame=1;frame<count;++frame) {
        jx_set_word(c,(uint16_t)(0x889c + (uint16_t)npc*86u+0x3c),(uint16_t)frame);
        (void)JX_CALL(c,0x1f2a,0xca66,(uint16_t)npc);
        (void)JX_CALL(c,0x1f2a,0x2dbc,4);
    }
}

/* 1F2A:DBA6. Reverse includes frame zero, unlike the forward animation. */
void jx_npc_reverse_animation(JxEventContext *c,int16_t npc) {
    int count=jx_signed_word(jx_word(c,(uint16_t)(0x31ce + (uint16_t)npc*18u+6)));
    if(count==0) return;
    /* Explicit word wrapping retains the original behavior for count 32767. */
    for(uint16_t step=1;jx_signed_word(step)<=count;step=(uint16_t)(step+1)) {
        jx_set_word(c,(uint16_t)(0x889c + (uint16_t)npc*86u+0x3c),(uint16_t)(count-step));
        (void)JX_CALL(c,0x1f2a,0xca66,(uint16_t)npc);
        (void)JX_CALL(c,0x1f2a,0x2dbc,4);
    }
}

/* 2F28:4384. Select EMS bank(s) and return offset of a 16x16 tile. */
uint16_t jx_tile_map_resource(JxEventContext *c,uint16_t kind,uint16_t index) {
    switch(kind) {
    case 0:
    case 1: {
        int signed_index=jx_signed_word(index);
        int page=signed_index>=0 ? signed_index/64 : -((-signed_index+63)/64);
        (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)(page+(kind==0 ? 0x12 : 0x22)),jx_word(c,0x9332));
        return (uint16_t)((index&63u)*256u);
    }
    case 2:
        for(uint16_t page=0;page<4;++page)
            (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(0x22 + page),jx_word(c,0x9332));
        return (uint16_t)(index*256u);
    case 3:
        (void)JX_CALL(c,0x1503,0x007d,0,0x3e,jx_word(c,0x9332));
        return (uint16_t)(index*256u);
    default:
        return kind; /* Observed AX fallthrough, no invented validation. */
    }
}

/* 2F28:4FDC. Compose up to eight tile layers and draw one viewport cell. */
static JxWorldUpdateFlow scene_draw_tile(JxEventContext *c,int16_t column,int16_t row,const JxFarPointer *scratch) {
    if(column<0 || column>19 || row<0 || row>11) return JX_WORLD_UPDATE_RETURNED;
    uint16_t x=(uint16_t)(column*16),y=(uint16_t)(row*16);
    if(jx_word(c,0x9362)!=0) {
        (void)JX_CALL(c,0x1832,0x1810,3);
        (void)JX_CALL(c,0x1832,0x1250,x,y,16,16,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    }
    if(scratch) {
        JxWorldUpdateFlow flow=jx_scene_cell_layers_world(c,(uint16_t)column,(uint16_t)row,*scratch);
        if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
    } else (void)JX_CALL(c,0x2f28,0x492a,(uint16_t)column,(uint16_t)row);
    int transparent=jx_word(c,0x9362)!=0;
    uint16_t offset=(uint16_t)JX_CALL(c,0x2f28,0x4384,jx_word(c,0x9384),jx_word(c,0x9386));
    uint16_t source=(uint16_t)(jx_word(c,0x5858)+offset);
    if(transparent) {
        (void)JX_CALL(c,0x1832,0x185e,jx_word(c,0x2eb0),jx_word(c,0x2eb2),source,jx_word(c,0x585a),256,255);
    } else {
        (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x2eb0),jx_word(c,0x2eb2),source,jx_word(c,0x585a),256);
    }
    for(uint16_t layer=1;layer<8;++layer) {
        uint16_t slot=(uint16_t)(0x9384 + layer*4);
        if(jx_word(c,slot)==0xffff) continue;
        offset=(uint16_t)JX_CALL(c,0x2f28,0x4384,jx_word(c,slot),jx_word(c,(uint16_t)(slot+2)));
        source=(uint16_t)(jx_word(c,0x5858)+offset);
        (void)JX_CALL(c,0x1832,0x185e,jx_word(c,0x2eb0),jx_word(c,0x2eb2),source,jx_word(c,0x585a),256,255);
    }
    (void)JX_CALL(c,0x1832,0x13b0,x,y,16,16,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    return JX_WORLD_UPDATE_RETURNED;
}
void jx_scene_draw_tile(JxEventContext *c,int16_t column,int16_t row) {
    (void)scene_draw_tile(c,column,row,NULL);
}
JxCellFlow jx_scene_draw_tile_cells(JxEventContext *c,int16_t column,int16_t row,JxFarPointer scratch) {
    return scene_draw_tile(c,column,row,&scratch)==JX_WORLD_UPDATE_RETURNED?JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;
}

JxWorldUpdateFlow jx_scene_redraw_world(JxEventContext *c,JxFarPointer scratch) {
    return scene_redraw(c,&scratch);
}
JxWorldUpdateFlow jx_scene_draw_tile_world(JxEventContext *c,int16_t column,int16_t row,JxFarPointer scratch) {
    return scene_draw_tile(c,column,row,&scratch);
}
