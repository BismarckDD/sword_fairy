#include "event_engine.h"
/* 1F2A:C264. Four-byte workspace replaces the stack index-offset local.
 * Its incoming bytes matter if an index read is short, as in the original. */
static uint32_t sprite_open(JxEventContext *c,uint16_t name,uint16_t mode) {
    return JX_CALL(c,0x1000,0x0738,name,c->data_segment,mode,c->data_segment);
}
static void sprite_close(JxEventContext *c,uint32_t stream) {
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)stream,(uint16_t)(stream>>16));
}
static void sprite_seek(JxEventContext *c,uint32_t stream,uint32_t offset,uint16_t origin) {
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)stream,(uint16_t)(stream>>16),(uint16_t)offset,(uint16_t)(offset>>16),origin);
}
static void sprite_read(JxEventContext *c,uint32_t stream,JxFarPointer target,uint16_t size,uint16_t count) {
    (void)JX_CALL(c,0x1000,0x0754,target.offset,target.segment,size,count,(uint16_t)stream,(uint16_t)(stream>>16));
}
static void sprite_next(JxEventContext *c,uint16_t slot,uint16_t value) {
    uint16_t next=(uint16_t)(0x31ce + 18*(uint16_t)(slot+1)+14);
    jx_set_word(c,next,value);jx_set_word(c,(uint16_t)(next+2),jx_signed_word(value)<0?0xffff:0);
}
static uint32_t sprite_offset(JxEventContext *c,JxFarPointer work) {
    return (uint32_t)jx_far_word(c,work.segment,work.offset)|((uint32_t)jx_far_word(c,work.segment,(uint16_t)(work.offset+2))<<16);
}
void jx_sprite_load(JxEventContext *c,uint16_t slot,uint16_t resource,JxFarPointer work) {
    if(jx_signed_word(resource)<0) {sprite_next(c,slot,jx_word(c,0x2b58));return;}
    uint32_t index=sprite_open(c,0x0880,0x087d);
    if(!index) {sprite_next(c,slot,jx_word(c,0x2b58));return;}
    sprite_seek(c,index,(uint32_t)((int32_t)jx_signed_word(resource)*4),0);
    sprite_read(c,index,work,4,1);sprite_close(c,index);
    uint32_t data=sprite_open(c,0x088b,0x0888);
    if(!data) {sprite_next(c,slot,jx_word(c,0x2b58));return;}
    sprite_seek(c,data,sprite_offset(c,work),0);
    uint16_t descriptor=(uint16_t)(0x31ce + 18*slot);
    sprite_read(c,data,(JxFarPointer){descriptor,c->data_segment},2,7);
    sprite_seek(c,data,4,1);
    uint16_t area=(uint16_t)((uint32_t)jx_word(c,descriptor)*jx_word(c,(uint16_t)(descriptor+2)));
    uint16_t alternate=(uint16_t)((uint32_t)area*jx_word(c,(uint16_t)(descriptor+6)));
    alternate=(uint16_t)((uint32_t)alternate*jx_word(c,(uint16_t)(descriptor+10)));
    uint16_t normal=(uint16_t)((uint32_t)area*jx_word(c,(uint16_t)(descriptor+4)));
    normal=(uint16_t)((uint32_t)normal*jx_word(c,(uint16_t)(descriptor+8)));
    uint16_t count=(uint16_t)(alternate+normal);
    (void)JX_CALL(c,0x1503,0x007d,0,67,jx_word(c,0x9332));
    JxFarPointer tile={(uint16_t)(jx_word(c,0x5858)+2*jx_word(c,0x2b58)),jx_word(c,0x585a)};
    for(int i=0;i<jx_signed_word(count);i++) {
        uint16_t value=(uint16_t)JX_CALL(c,0x1000,0x2a1a,(uint16_t)data,(uint16_t)(data>>16));
        /* Original stores then adds the live base: preserve even DS aliasing. */
        uint32_t at=((uint32_t)tile.segment*16+tile.offset)&0xfffff;
        c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
        value=(uint16_t)(jx_far_word(c,tile.segment,tile.offset)+jx_word(c,0x2b58));
        c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
        tile.offset=(uint16_t)(tile.offset+2);
    }
    sprite_next(c,slot,(uint16_t)(jx_word(c,0x2b58)+count));sprite_close(c,data);
    index=sprite_open(c,0x0896,0x0893);
    if(!index) {sprite_next(c,slot,jx_word(c,0x2b58));return;}
    sprite_seek(c,index,(uint32_t)((int32_t)jx_signed_word(resource)*4),0);
    sprite_read(c,index,work,4,1);sprite_close(c,index);
    data=sprite_open(c,0x08a1,0x089e);
    if(!data) {sprite_next(c,slot,jx_word(c,0x2b58));return;}
    sprite_seek(c,data,sprite_offset(c,work),0);sprite_seek(c,data,6,1);
    for(int i=0;i<jx_signed_word(count);i++) {
        int index_value=jx_signed_word((uint16_t)(jx_word(c,0x2b58)+i));
        (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)(index_value/64+34),jx_word(c,0x9332));
        index_value=jx_signed_word((uint16_t)(jx_word(c,0x2b58)+i));
        JxFarPointer target={(uint16_t)(jx_word(c,0x5858)+(index_value%64)*256),jx_word(c,0x585a)};
        sprite_read(c,data,target,1,256);
    }
    sprite_close(c,data);jx_set_word(c,0x2b58,(uint16_t)(jx_word(c,0x2b58)+count));
}
