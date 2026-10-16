#include "event_engine.h"
static JxWorldUpdateFlow party_draw_tile(JxEventContext *c,uint16_t x,uint16_t y,const JxFarPointer *scratch) {
    if(scratch) return jx_scene_draw_tile_world(c,(int16_t)jx_signed_word(x),(int16_t)jx_signed_word(y),*scratch);
    (void)JX_CALL(c,0x2f28,0x4fdc,x,y);return JX_WORLD_UPDATE_RETURNED;
}
static JxWorldUpdateFlow party_full_redraw(JxEventContext *c,const JxFarPointer *scratch) {
    if(scratch) return jx_scene_redraw_world(c,*scratch);
    (void)JX_CALL(c,0x2f28,0x5580);return JX_WORLD_UPDATE_RETURNED;
}
/* 2F28:8C6E. Ping-pong walking frames skip standing frame 2. */
void jx_party_advance_frames(JxEventContext *c) {
    for(uint16_t i=0;i<3;i++) {
        uint16_t state=(uint16_t)(0x0e14+2*i),slot=(uint16_t)(0x8784+2*i),frame=jx_word(c,slot);
        if(jx_word(c,state)==1) {
            frame++;
            if(jx_signed_word(frame)>4) {frame=3;jx_set_word(c,state,0);}
            if(frame==2) frame=3;
        } else {
            frame--;
            if(jx_signed_word(frame)<0) {frame=1;jx_set_word(c,state,1);}
            if(frame==2) frame=1;
        }
        jx_set_word(c,slot,frame);
    }
}
/* 2F28:8D50. Six trail samples, three words each; copy backwards. */
void jx_party_shift_trail(JxEventContext *c) {
    for(uint16_t i=5;i>0;i--) for(uint16_t field=0;field<6;field+=2)
        jx_set_word(c,(uint16_t)(0x8760+6*i+field),jx_word(c,(uint16_t)(0x8760+6*(i-1)+field)));
}
/* 2F28:5166. Redraw both trail samples for each enabled follower. Camera,
 * coordinates and subsequent follower membership remain live. */
static JxWorldUpdateFlow party_redraw_trail(JxEventContext *c,const JxFarPointer *scratch) {
    for(uint16_t member=0;member<3;member++) {
        if(member && jx_signed_word(jx_word(c,(uint16_t)(0x875a+2*member)))<0) continue;
        for(uint16_t sample=(uint16_t)(member*2);sample<member*2+2;sample++)
            for(uint16_t row=0;row<3;row++) {
                uint16_t actor=(uint16_t)(0x8760+6*sample);
                uint16_t y=(uint16_t)(jx_word(c,(uint16_t)(actor+2))-jx_word(c,0x8898)-row);
                uint16_t x=(uint16_t)(jx_word(c,actor)-jx_word(c,0x8896));
                JxWorldUpdateFlow flow=party_draw_tile(c,x,y,scratch);
                if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
            }
    }
    return JX_WORLD_UPDATE_RETURNED;
}
/* 2F28:8DD6. Camera clamp comparisons deliberately differ by travel sign:
 * negative travel tests <0 then >=limit; positive tests >limit then <=0. */
static JxWorldUpdateFlow party_move_cycle(JxEventContext *c,uint16_t facing,uint16_t direction,const JxFarPointer *scratch) {
    jx_party_shift_trail(c);jx_set_word(c,0x8764,facing);
    if(direction<4) {
        uint16_t coordinate=direction<2?0x8762:0x8760,camera=direction<2?0x8898:0x8896;
        int positive=direction&1;
        jx_set_word(c,coordinate,(uint16_t)(jx_word(c,coordinate)+(positive?1:-1)));
        jx_party_advance_frames(c);
        if(jx_word(c,0x0ddc)) {
            if(scratch) return jx_party_redraw_page_world(c,*scratch);
            (void)JX_CALL(c,0x2f28,0x5346);return JX_WORLD_UPDATE_RETURNED;
        }
        jx_set_word(c,camera,(uint16_t)(jx_word(c,coordinate)-(direction<2?6:9)));
        int current=jx_signed_word(jx_word(c,camera));
        uint16_t limit=(uint16_t)(jx_word(c,direction<2?0x663c:0x663a)-(direction<2?12:20));
        int redraw=1;
        if(!positive) {
            if(current<0) jx_set_word(c,camera,0);
            else if(current>=jx_signed_word(limit)) jx_set_word(c,camera,limit);
            else redraw=0;
        } else {
            if(current>jx_signed_word(limit)) jx_set_word(c,camera,limit);
            else if(current<=0) jx_set_word(c,camera,0);
            else redraw=0;
        }
        if(scratch) {
            JxWorldUpdateFlow flow=redraw?jx_party_redraw_page_world(c,*scratch):jx_party_scroll_page_world(c,direction,*scratch);
            if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
        } else if(redraw) (void)JX_CALL(c,0x2f28,0x5346);
        else (void)JX_CALL(c,0x2f28,0x53b0,direction);
    }
    if(scratch)return jx_party_check_triggers_world(c,*scratch);
    (void)JX_CALL(c,0x2f28,0x88de);
    return JX_WORLD_UPDATE_RETURNED;
}
static uint16_t party_page(JxEventContext *c,uint16_t next) {
    return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+next))%2);
}
/* 2F28:5346. Copy the previous page, repaint the trail and present. */
static JxWorldUpdateFlow party_redraw_page(JxEventContext *c,const JxFarPointer *scratch) {
    if(jx_word(c,0x9362)) return party_full_redraw(c,scratch);
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    (void)JX_CALL(c,0x1832,0x135e,party_page(c,0),party_page(c,1));
    (void)JX_CALL(c,0x1832,0x1810,party_page(c,0));
    JxWorldUpdateFlow flow=party_redraw_trail(c,scratch);
    if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
    (void)JX_CALL(c,0x1832,0x1836,party_page(c,0));
    return JX_WORLD_UPDATE_RETURNED;
}
/* 2F28:53B0. One complete 16-pixel tile shift, not smooth scrolling. */
static JxWorldUpdateFlow party_scroll_page(JxEventContext *c,uint16_t direction,const JxFarPointer *scratch) {
    if(jx_word(c,0x9362)) return party_full_redraw(c,scratch);
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    (void)JX_CALL(c,0x1832,0x1810,party_page(c,0));
    if(direction<4) {
        uint16_t source_x=direction==3?16:0,source_y=direction==1?16:0;
        uint16_t destination_x=direction==2?16:0,destination_y=direction==0?16:0;
        (void)JX_CALL(c,0x1832,0x11a8,source_x,source_y,direction<2?320:304,direction<2?176:192,
            destination_x,destination_y,party_page(c,0),party_page(c,1));
        for(uint16_t i=0;i<(direction<2?20:12);i++) {
            JxWorldUpdateFlow flow=party_draw_tile(c,direction<2?i:(direction==2?0:19),direction<2?(direction==0?0:11):i,scratch);
            if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
        }
        JxWorldUpdateFlow flow=party_redraw_trail(c,scratch);
    if(flow!=JX_WORLD_UPDATE_RETURNED) return flow;
    }
    (void)JX_CALL(c,0x1832,0x1836,party_page(c,0));
    return JX_WORLD_UPDATE_RETURNED;
}

void jx_party_redraw_trail(JxEventContext *c) {(void)party_redraw_trail(c,NULL);}
JxCellFlow jx_party_redraw_trail_cells(JxEventContext *c,JxFarPointer scratch) {return party_redraw_trail(c,&scratch)==JX_WORLD_UPDATE_RETURNED?JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;}
void jx_party_redraw_page(JxEventContext *c) {(void)party_redraw_page(c,NULL);}
JxCellFlow jx_party_redraw_page_cells(JxEventContext *c,JxFarPointer scratch) {return party_redraw_page(c,&scratch)==JX_WORLD_UPDATE_RETURNED?JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;}
void jx_party_scroll_page(JxEventContext *c,uint16_t direction) {(void)party_scroll_page(c,direction,NULL);}
JxCellFlow jx_party_scroll_page_cells(JxEventContext *c,uint16_t direction,JxFarPointer scratch) {return party_scroll_page(c,direction,&scratch)==JX_WORLD_UPDATE_RETURNED?JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;}

void jx_party_move_cycle(JxEventContext *c,uint16_t facing,uint16_t direction) {(void)party_move_cycle(c,facing,direction,NULL);}
JxCellFlow jx_party_move_cycle_cells(JxEventContext *c,uint16_t facing,uint16_t direction,JxFarPointer scratch) {return party_move_cycle(c,facing,direction,&scratch)==JX_WORLD_UPDATE_RETURNED?JX_CELL_RETURNED:JX_CELL_DIVIDE_ERROR;}

JxWorldUpdateFlow jx_party_redraw_trail_world(JxEventContext *c,JxFarPointer scratch) {return party_redraw_trail(c,&scratch);}

JxWorldUpdateFlow jx_party_redraw_page_world(JxEventContext *c,JxFarPointer scratch) {return party_redraw_page(c,&scratch);}

JxWorldUpdateFlow jx_party_scroll_page_world(JxEventContext *c,uint16_t direction,JxFarPointer scratch) {return party_scroll_page(c,direction,&scratch);}

JxWorldUpdateFlow jx_party_move_cycle_world(JxEventContext *c,uint16_t facing,uint16_t direction,JxFarPointer scratch) {return party_move_cycle(c,facing,direction,&scratch);}
