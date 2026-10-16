#include "event_engine.h"

/* 2F28:866A. Query CD track range; a 22-track difference is accepted. */
uint16_t jx_cd_validate_track_range(JxEventContext *c,JxFarPointer work) {
    if((uint16_t)JX_CALL(c,0x1832,0x21d8,work.offset,work.segment)!=0)return 0xffff;
    int first=jx_far_byte(c,work,0),last=jx_far_byte(c,work,1);
    return last-first==22?0:1;
}

/* 1F2A:DF00. Periodic encounter; companion is captured before RNG. */
void jx_world_random_encounter(JxEventContext *c) {
    uint16_t companion=0xffff;
    int second=jx_signed_word(jx_word(c,0x875c));
    if(second>=0 && second<4)companion=1;
    if(jx_word(c,0x585e)%20!=0)return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x71de,10)!=1)return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,0,0,companion)==1)
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
}

/* 2F28:9FFE. Two moving scene hazards, tested against the cell above. */
void jx_world_moving_hazards(JxEventContext *c) {
    for(uint16_t i=15;i<17;i++){
        uint16_t definition=(uint16_t)(0x31ce + 18*i),npc=(uint16_t)(0x889c+86*i);
        uint16_t width=jx_word(c,definition),height=jx_word(c,(uint16_t)(definition+2));
        uint16_t left=jx_word(c,(uint16_t)(npc+64));
        uint16_t top=(uint16_t)(jx_word(c,(uint16_t)(npc+66))-height+1),col,row;
        if(jx_rectangle_local_point(&col,&row,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)-1),
            left,top,jx_signed_word(width),jx_signed_word(height))!=0)continue;
        if(jx_signed_word(jx_word(c,(uint16_t)(npc+60)))%7>=3)continue;
        (void)JX_CALL(c,0x1f2a,0x5fd8,36,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
        (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x0e3e,c->data_segment,0);
        (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
        return;
    }
}

static JxFarPointer callback_at(JxFarPointer work,uint16_t field){
    return (JxFarPointer){(uint16_t)(work.offset+field),work.segment};
}
static void callback_store(JxEventContext *c,JxFarPointer work,uint16_t field,uint16_t value){
    JxFarPointer p=callback_at(work,field);
    uint32_t at=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
static uint16_t callback_word(JxEventContext *c,JxFarPointer work,uint16_t field){
    JxFarPointer p=callback_at(work,field);return jx_far_word(c,p.segment,p.offset);
}
/* Preserve the original local tables in caller-owned emulated work, including
 * out-of-range facing indices that can read other original frame words. */
static void callback_tile_table(JxEventContext *c,JxFarPointer work,uint16_t start,
    const uint16_t *tiles,unsigned count){
    for(unsigned i=0;i<count;i++)callback_store(c,work,(uint16_t)(start+2*i),tiles[i]);
}
static int callback_query(JxEventContext *c,JxFarPointer work,uint16_t image_field,
    uint16_t table_field,unsigned count){
    JxFarPointer layer=callback_at(work,6),image=callback_at(work,image_field);
    (void)JX_CALL(c,0x2f28,0x448a,layer.offset,layer.segment,image.offset,image.segment,
        jx_word(c,0x8760),jx_word(c,0x8762),0);
    callback_store(c,work,(uint16_t)(image_field-2),0);
    for(unsigned i=0;i<count;i++){
        callback_store(c,work,(uint16_t)(image_field-2),(uint16_t)i);
        if(callback_word(c,work,(uint16_t)(table_field+2*i))!=callback_word(c,work,image_field))continue;
        int direction=jx_signed_word(jx_word(c,0x8764))/2;
        uint16_t replacement=callback_word(c,work,(uint16_t)(2+2*(1-direction)));
        callback_store(c,work,0,replacement);
        uint16_t offset=(uint16_t)(jx_word(c,0x5858)+2*jx_word(c,0x8762)*jx_word(c,0x663a)+2*jx_word(c,0x8760));
        callback_store(c,(JxFarPointer){offset,jx_word(c,0x585a)},0,callback_word(c,work,0));
        return 1;
    }
    callback_store(c,work,(uint16_t)(image_field-2),(uint16_t)count);
    return 0;
}
/* 2F28:9C3A, 50-byte original frame. Return1 represents DIV zero fault. */
uint16_t jx_world_direction_tiles(JxEventContext *c,JxFarPointer work){
    static const uint16_t tiles[18]={2,3,6,7,17,18,23,28,29,34,38,40,41,48,49,51,59,60};
    callback_tile_table(c,work,8,tiles,18);
    callback_store(c,work,2,0xffc2);callback_store(c,work,4,0xffb7);
    uint16_t period=jx_word(c,0x0e10);if(!period)return 1;
    if(jx_word(c,0x585e)%period==0)(void)callback_query(c,work,46,8,18);
    return 0;
}
/* 2F28:7886 / 7934. Original alternating page shift and busy wait. */
void jx_world_shift_page(JxEventContext *c,uint16_t height,uint16_t delay){
    uint16_t counter=jx_word(c,0x3e54);
    (void)JX_CALL(c,0x1832,0x11a8,0,height,320,(uint16_t)(200-height),0,0,
        (uint16_t)(jx_signed_word((uint16_t)(counter+1))%2),(uint16_t)(jx_signed_word(counter)%2));
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2));
    (void)JX_CALL(c,0x1832,0x1884,0,(uint16_t)(192-height),320,height,0);
    (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2));
    (void)JX_CALL(c,0x1f2a,0x720e,delay);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
}
void jx_world_shift_pages(JxEventContext *c,uint16_t height,uint16_t delay){
    if(jx_signed_word(height)<=0 || jx_signed_word(delay)<=0)return;
    for(uint16_t i=height;jx_signed_word(i)>0;i=(uint16_t)(i-1))
        (void)JX_CALL(c,0x2f28,0x7886,i,delay);
}
/* 2F28:9D42, 60-byte original frame. */
uint16_t jx_world_sinking_tiles(JxEventContext *c,JxFarPointer work){
    static const uint16_t tiles[18]={6,7,17,18,23,28,29,34,40,41,51,374,375,410,420,421,431,432};
    callback_tile_table(c,work,8,tiles,18);
    JxFarPointer empty=callback_at(work,44);
    (void)JX_CALL(c,0x1000,0x3572,empty.offset,empty.segment,0,10);
    callback_store(c,work,2,0xffc2);callback_store(c,work,4,0xffb7);
    uint16_t period=jx_word(c,0x93a8);if(!period)return 1;
    if(jx_word(c,0x585e)%period==0){
        (void)JX_CALL(c,0x2f28,0x7934,10,20);
        if(jx_signed_word(jx_word(c,0x93a8))>20)jx_set_word(c,0x93a8,(uint16_t)(jx_word(c,0x93a8)-1));
        if(jx_signed_word(jx_word(c,0x93a8))<25){
            (void)JX_CALL(c,0x1f2a,0x5fd8,58,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
            (void)JX_CALL(c,0x2f28,0x7062,0x0e1c,c->data_segment,7,0);
            (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x0e28,c->data_segment,0);
            (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
            while((uint16_t)JX_CALL(c,0x1f2a,0xa2f0)!=0){}
        }
        return 0;
    }
    period=jx_word(c,0x0e10);if(!period)return 1;
    if(jx_word(c,0x585e)%period==0)(void)callback_query(c,work,56,8,18);
    return 0;
}
/* 2F28:9EFE, 34-byte original frame. */
uint16_t jx_world_lethal_tiles(JxEventContext *c,JxFarPointer work){
    static const uint16_t tiles[13]={10,20,21,22,31,32,33,42,43,44,53,54,55};
    callback_tile_table(c,work,2,tiles,13);
    uint16_t period=jx_word(c,0x0e10);if(!period)return 1;
    if(jx_word(c,0x585e)%period!=0)return 0;
    JxFarPointer image=callback_at(work,30);
    (void)JX_CALL(c,0x2f28,0x448a,work.offset,work.segment,image.offset,image.segment,
        jx_word(c,0x8760),jx_word(c,0x8762),0);
    callback_store(c,work,28,0);
    for(unsigned i=0;i<13;i++){
        callback_store(c,work,28,(uint16_t)i);
        if(callback_word(c,work,(uint16_t)(2+2*i))!=callback_word(c,work,30))continue;
        (void)JX_CALL(c,0x1f2a,0x5fd8,6,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
        jx_set_word(c,0x875a,0xffff);(void)JX_CALL(c,0x2f28,0x5580);
        (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x0e33,c->data_segment,0);
        (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
        while((uint16_t)JX_CALL(c,0x1f2a,0xa2f0)!=0){}
        return 0;
    }
    callback_store(c,work,28,13);
    return 0;
}
