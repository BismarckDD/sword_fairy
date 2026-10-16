#include "event_engine.h"
/* Native far word stores continue physically beyond offset FFFF. */
static void cell_store(JxEventContext *c,JxFarPointer p,uint16_t value) {
    uint32_t address=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[address]=(uint8_t)value;c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
}
static uint16_t map_cell(JxEventContext *c,uint16_t x,uint16_t y) {
    uint16_t offset=(uint16_t)(jx_word(c,0x5858)+2*x+2u*y*jx_word(c,0x663a));
    return jx_far_word(c,jx_word(c,0x585a),offset);
}
/* 2F28:448A. Background/foreground map lookup. Output order matters when
 * pointers alias the map metadata: the second tile is deliberately reread. */
uint16_t jx_map_cell_info(JxEventContext *c,JxFarPointer layer,JxFarPointer image,uint16_t x,uint16_t y,uint16_t foreground) {
    uint16_t logical=foreground?83:63;
    for(uint16_t page=0;page<4;page++)
        (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(logical+page),jx_word(c,0x9332));
    cell_store(c,image,(uint16_t)(jx_absolute_word(map_cell(c,x,y))&0x0fff));
    cell_store(c,layer,(uint16_t)((jx_absolute_word(map_cell(c,x,y))&0x7000)>>12));
    return 0;
}
/* Shared body of 2F28:45E4 and 475C. Descriptor indexing and products wrap
 * at 16 bits BEFORE signed extension for huge-pointer arithmetic. */
static uint16_t actor_cell_info(JxEventContext *c,JxFarPointer layer,JxFarPointer image,
        uint16_t column,uint16_t row,uint16_t actor,uint16_t direction,uint16_t frame,int alternate) {
    for(uint16_t page=0;page<4;page++) {
        uint16_t status=(uint16_t)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(67+page),jx_word(c,0x9332));
        if(status) return status;
    }
    uint16_t descriptor=(uint16_t)(0x31ce + actor*18u);
    uint16_t width=jx_word(c,descriptor),area=(uint16_t)((uint32_t)width*jx_word(c,(uint16_t)(descriptor+2)));
    uint16_t extra=alternate?(uint16_t)((uint32_t)area*jx_word(c,(uint16_t)(descriptor+4))*jx_word(c,(uint16_t)(descriptor+8))):0;
    if(jx_word(c,(uint16_t)(descriptor+(alternate?10:8)))==1) direction=0;
    uint16_t index=(uint16_t)((uint32_t)area*jx_word(c,(uint16_t)(descriptor+(alternate?6:4)))*direction+(uint32_t)area*frame+(uint32_t)width*row+column);
    uint32_t base=(uint32_t)jx_word(c,(uint16_t)(descriptor+14))|((uint32_t)jx_word(c,(uint16_t)(descriptor+16))<<16);
    uint32_t address=((uint32_t)jx_word(c,0x585a)*16+jx_word(c,0x5858)+2*base+
        (uint32_t)(2*jx_signed_word(extra))+(uint32_t)(2*jx_signed_word(index)))&0xfffff;
    uint16_t tile=(uint16_t)(c->memory[address]|((uint16_t)c->memory[(address+1)&0xfffff]<<8));
    cell_store(c,layer,(uint16_t)((tile&0x7000)>>12));
    uint16_t result=(uint16_t)(jx_absolute_word(tile)&0x0fff);
    cell_store(c,image,result);return result;
}
uint16_t jx_actor_cell_info(JxEventContext *c,JxFarPointer layer,JxFarPointer image,
        uint16_t column,uint16_t row,uint16_t actor,uint16_t direction,uint16_t frame) {
    return actor_cell_info(c,layer,image,column,row,actor,direction,frame,0);
}
uint16_t jx_actor_alternate_cell_info(JxEventContext *c,JxFarPointer layer,JxFarPointer image,
        uint16_t column,uint16_t row,uint16_t actor,uint16_t direction,uint16_t frame) {
    return actor_cell_info(c,layer,image,column,row,actor,direction,frame,1);
}
/* 2F28:492A. Compose nine layer slots for a camera-relative map cell.
 * Four private bytes preserve layer/image values across failed EMS mappings.
 * Divide traps are surfaced before any later scene work executes. */
JxWorldUpdateFlow jx_scene_cell_layers_world(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer scratch) {
    JxFarPointer image={(uint16_t)(scratch.offset+2),scratch.segment};
    for(uint16_t i=0;i<9;i++) jx_set_word(c,(uint16_t)(0x9384+4*i),0xffff);
    (void)jx_map_cell_info(c,scratch,image,(uint16_t)(jx_word(c,0x8896)+x),(uint16_t)(jx_word(c,0x8898)+y),0);
    jx_set_word(c,0x9384,0);jx_set_word(c,0x9386,jx_far_word(c,image.segment,image.offset));
    for(uint16_t i=0;i<3;i++) {
        if(jx_signed_word(jx_word(c,(uint16_t)(0x875a+2*i)))<0) continue;
        uint16_t actor=(uint16_t)(0x8760+12*i),column,row;
        if(jx_rectangle_local_point(&column,&row,(uint16_t)(jx_word(c,0x8896)+x),(uint16_t)(jx_word(c,0x8898)+y),
            jx_word(c,actor),(uint16_t)(jx_word(c,(uint16_t)(actor+2))-2),1,3)) continue;
        (void)jx_actor_cell_info(c,scratch,image,column,row,(uint16_t)(i+30),jx_word(c,(uint16_t)(actor+4)),jx_word(c,(uint16_t)(0x8784+2*i)));
        uint16_t slot=(uint16_t)(0x9384+4*jx_far_word(c,scratch.segment,scratch.offset));
        if(jx_signed_word(jx_word(c,slot))<0) {
            jx_set_word(c,slot,1);jx_set_word(c,(uint16_t)(slot+2),jx_far_word(c,image.segment,image.offset));
        }
    }
    for(uint16_t i=0;i<30;i++) {
        uint16_t presence=(uint16_t)(0x665a+30*jx_word(c,0x9334)+i),actor=(uint16_t)(0x889c+86*i);
        if(jx_ds(c)[presence]!=1 || jx_signed_word(jx_word(c,(uint16_t)(actor+14)))<0) continue;
        uint16_t descriptor=(uint16_t)(0x31ce + 18*i),width=jx_word(c,descriptor),height=jx_word(c,(uint16_t)(descriptor+2)),column,row;
        if(jx_rectangle_local_point(&column,&row,(uint16_t)(jx_word(c,0x8896)+x),(uint16_t)(jx_word(c,0x8898)+y),
            jx_word(c,(uint16_t)(actor+0x40)),(uint16_t)(jx_word(c,(uint16_t)(actor+0x42))-height+1),
            (int16_t)jx_signed_word(width),(int16_t)jx_signed_word(height))) continue;
        int alternate=jx_word(c,(uint16_t)(actor+0x1a))!=0;
        int divisor=jx_signed_word(jx_word(c,(uint16_t)(descriptor+(alternate?6:4))));
        int frame=jx_signed_word(jx_word(c,(uint16_t)(actor+(alternate?0x3c:0x3e))));
        if(!divisor || (frame==-32768 && divisor==-1))
            return alternate?JX_WORLD_SCENE_ACTIVE_DIVIDE_ERROR:JX_WORLD_SCENE_IDLE_DIVIDE_ERROR;
        uint16_t remainder=(uint16_t)(frame%divisor),direction=jx_word(c,(uint16_t)(actor+0x44));
        if(alternate) (void)jx_actor_alternate_cell_info(c,scratch,image,column,row,i,direction,remainder);
        else (void)jx_actor_cell_info(c,scratch,image,column,row,i,direction,remainder);
        uint16_t slot=(uint16_t)(0x9384+4*jx_far_word(c,scratch.segment,scratch.offset));
        if(jx_signed_word(jx_word(c,slot))<0) {
            jx_set_word(c,slot,1);jx_set_word(c,(uint16_t)(slot+2),jx_far_word(c,image.segment,image.offset));
        }
    }
    (void)jx_map_cell_info(c,scratch,image,(uint16_t)(jx_word(c,0x8896)+x),(uint16_t)(jx_word(c,0x8898)+y),1);
    uint16_t layer=(uint16_t)(jx_far_word(c,scratch.segment,scratch.offset)&7);
    if(!layer) layer=4;
    cell_store(c,scratch,layer);
    uint16_t tile=jx_far_word(c,image.segment,image.offset);
    if(tile!=1) {jx_set_word(c,(uint16_t)(0x9384+4*layer),0);jx_set_word(c,(uint16_t)(0x9386+4*layer),tile);}
    if(jx_word(c,0x9362)) return JX_WORLD_UPDATE_RETURNED;
    for(uint16_t i=0;i<10;i++) {
        uint16_t object=(uint16_t)(0x92b8+10*i);
        if(jx_word(c,object)!=(uint16_t)(jx_word(c,0x8896)+x) || jx_word(c,(uint16_t)(object+2))!=(uint16_t)(jx_word(c,0x8898)+y)) continue;
        int type=jx_signed_word(jx_word(c,(uint16_t)(object+4)));
        if(type<0 || type>5) continue;
        jx_set_word(c,0x9388,3);
        uint16_t state=(uint16_t)(0xacec+10*jx_word(c,0x9334)+i);
        jx_set_word(c,0x938a,(uint16_t)(2*type+jx_signed_byte(jx_ds(c)[state])));
    }
    return JX_WORLD_UPDATE_RETURNED;
}
/* Preserve the earlier rendering ABI's single fault result. */
JxCellFlow jx_scene_cell_layers(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer scratch) {
    return jx_scene_cell_layers_world(c,x,y,scratch)==JX_WORLD_UPDATE_RETURNED?
        JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;
}


/* 2F28:4D12. Collision layers omit party poses and the drawing-only
 * object gates in492A; exact NPC divide errors propagate before foreground. */
JxWorldUpdateFlow jx_scene_collision_layers(JxEventContext *c,
        uint16_t x,uint16_t y,JxFarPointer scratch) {
    JxFarPointer image={(uint16_t)(scratch.offset+2),scratch.segment};
    for(uint16_t i=0;i<9;i++)jx_set_word(c,(uint16_t)(0x9384+4*i),0xffff);
    (void)jx_map_cell_info(c,scratch,image,(uint16_t)(jx_word(c,0x8896)+x),
        (uint16_t)(jx_word(c,0x8898)+y),0);
    jx_set_word(c,0x9384,0);
    jx_set_word(c,0x9386,jx_far_word(c,image.segment,image.offset));
    for(uint16_t i=0;i<30;i++) {
        uint16_t presence=(uint16_t)(0x665a+30u*jx_word(c,0x9334)+i);
        uint16_t actor=(uint16_t)(0x889c+86u*i);
        if(jx_ds(c)[presence]!=1 || jx_signed_word(jx_word(c,(uint16_t)(actor+14)))<0)continue;
        uint16_t descriptor=(uint16_t)(0x31ce + 18u*i);
        uint16_t width=jx_word(c,descriptor),height=jx_word(c,(uint16_t)(descriptor+2));
        uint16_t column,row;
        if(jx_rectangle_local_point(&column,&row,(uint16_t)(jx_word(c,0x8896)+x),
            (uint16_t)(jx_word(c,0x8898)+y),jx_word(c,(uint16_t)(actor+0x40)),
            (uint16_t)(jx_word(c,(uint16_t)(actor+0x42))-height+1),
            (int16_t)jx_signed_word(width),(int16_t)jx_signed_word(height)))continue;
        int alternate=jx_word(c,(uint16_t)(actor+0x1a))!=0;
        int divisor=jx_signed_word(jx_word(c,(uint16_t)(descriptor+(alternate?6:4))));
        int frame=jx_signed_word(jx_word(c,(uint16_t)(actor+(alternate?0x3c:0x3e))));
        if(!divisor || (frame==-32768 && divisor==-1))
            return alternate?JX_WORLD_COLLISION_ACTIVE_DIVIDE_ERROR:JX_WORLD_COLLISION_IDLE_DIVIDE_ERROR;
        uint16_t remainder=(uint16_t)(frame%divisor);
        uint16_t direction=jx_word(c,(uint16_t)(actor+0x44));
        if(alternate)(void)jx_actor_alternate_cell_info(c,scratch,image,column,row,i,direction,remainder);
        else (void)jx_actor_cell_info(c,scratch,image,column,row,i,direction,remainder);
        uint16_t slot=(uint16_t)(0x9384+4u*jx_far_word(c,scratch.segment,scratch.offset));
        if(jx_signed_word(jx_word(c,slot))<0) {
            jx_set_word(c,slot,1);
            jx_set_word(c,(uint16_t)(slot+2),jx_far_word(c,image.segment,image.offset));
        }
    }
    (void)jx_map_cell_info(c,scratch,image,(uint16_t)(jx_word(c,0x8896)+x),
        (uint16_t)(jx_word(c,0x8898)+y),1);
    uint16_t layer=(uint16_t)(jx_far_word(c,scratch.segment,scratch.offset)&7);
    if(!layer)layer=4;
    cell_store(c,scratch,layer);
    uint16_t tile=jx_far_word(c,image.segment,image.offset);
    if(tile!=1) {
        jx_set_word(c,(uint16_t)(0x9384+4*layer),0);
        jx_set_word(c,(uint16_t)(0x9386+4*layer),tile);
    }
    for(uint16_t i=0;i<10;i++) {
        uint16_t object=(uint16_t)(0x92b8+10*i);
        if(jx_word(c,object)!=(uint16_t)(jx_word(c,0x8896)+x) ||
            jx_word(c,(uint16_t)(object+2))!=(uint16_t)(jx_word(c,0x8898)+y))continue;
        jx_set_word(c,0x9388,3);
        uint16_t state=(uint16_t)(0xacec+10u*jx_word(c,0x9334)+i);
        jx_set_word(c,0x938a,(uint16_t)(2u*jx_word(c,(uint16_t)(object+4))+
            jx_signed_byte(jx_ds(c)[state])));
    }
    return JX_WORLD_UPDATE_RETURNED;
}
