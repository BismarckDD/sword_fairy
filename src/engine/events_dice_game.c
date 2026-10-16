#include "event_engine.h"
#include <string.h>
static JxFarPointer dg_at(JxFarPointer frame,uint16_t off) {return (JxFarPointer){(uint16_t)(frame.offset+off),frame.segment};}
static uint16_t dg_get(JxEventContext *c,JxFarPointer frame,uint16_t off) {JxFarPointer p=dg_at(frame,off);return jx_far_word(c,p.segment,p.offset);}
static void dg_set(JxEventContext *c,JxFarPointer frame,uint16_t off,uint16_t value) {
    JxFarPointer p=dg_at(frame,off);uint32_t at=((uint32_t)p.segment*16+p.offset)&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
static JxFarPointer dg_buffer(JxEventContext *c,uint16_t off) {
    uint32_t sum=(uint32_t)jx_word(c,0x2eb0)+off;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(jx_word(c,0x2eb2)+((sum>>16)<<12))};
}
static uint16_t dg_page(JxEventContext *c,uint16_t advance) {return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x2952)+advance))%2+2);}
static void dg_image(JxEventContext *c,uint16_t entry,uint16_t x,uint16_t y,uint16_t width,uint16_t height,JxFarPointer p) {
    (void)JX_CALL(c,0x1832,entry,x,y,width,height,p.offset,p.segment);
}
static void dg_face(JxEventContext *c,uint16_t die,uint16_t x,uint16_t y) {
    (void)JX_CALL(c,0x1832,0x1a28,x,y,16,20,(uint16_t)(jx_word(c,0x5858)+die*320u),jx_word(c,0x585a),255);
}
static int dg_map(JxEventContext *c,uint16_t first,uint16_t count) {
    for(uint16_t i=0;i<count;i++)if((uint16_t)JX_CALL(c,0x1503,0x7d,i,(uint16_t)(first+i),jx_word(c,0x9332)))return 0;
    return 1;
}
static uint32_t dg_file(JxEventContext *c,JxFarPointer frame) {return dg_get(c,frame,226)|((uint32_t)dg_get(c,frame,228)<<16);}
static void dg_open(JxEventContext *c,JxFarPointer frame,uint16_t name,uint16_t mode) {
    uint32_t file=JX_CALL(c,0x1000,0x738,name,c->data_segment,mode,c->data_segment);
    dg_set(c,frame,226,(uint16_t)file);dg_set(c,frame,228,(uint16_t)(file>>16));
}
static void dg_seek(JxEventContext *c,JxFarPointer frame) {
    (void)JX_CALL(c,0x1000,0x27fa,dg_get(c,frame,226),dg_get(c,frame,228),16,0,0);
}
static void dg_read(JxEventContext *c,JxFarPointer frame,JxFarPointer dest,uint16_t size,uint16_t count) {
    (void)JX_CALL(c,0x1000,0x754,dest.offset,dest.segment,size,count,dg_get(c,frame,226),dg_get(c,frame,228));
}
static void dg_close(JxEventContext *c,JxFarPointer frame) {(void)JX_CALL(c,0x1000,0x636,dg_get(c,frame,226),dg_get(c,frame,228));}
static void dg_palette(JxEventContext *c,uint16_t source) {
    uint64_t bits=0;for(unsigned i=0;i<8;i++)bits|=(uint64_t)jx_ds(c)[0x2398+i]<<(8*i);
    double original;float weight;uint32_t argument;_Static_assert(sizeof(original)==8 && sizeof(weight)==4,"IEEE floats required");
    memcpy(&original,&bits,8);weight=(float)original;memcpy(&argument,&weight,4);
    (void)JX_CALL(c,0x2f28,0xa8d0,source,c->data_segment,jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,(uint16_t)argument,(uint16_t)(argument>>16));
}
static void dg_text(JxEventContext *c,uint16_t x,uint16_t y,JxFarPointer text) {(void)JX_CALL(c,0x1f2a,0x728,x,y,text.offset,text.segment);}
static void dg_number(JxEventContext *c,JxFarPointer frame,uint16_t value,uint16_t format,uint16_t x) {
    (void)JX_CALL(c,0x1832,0x1884,x,120,32,20,255);
    JxFarPointer text=dg_at(frame,8);(void)JX_CALL(c,0x1000,0x2a8c,text.offset,text.segment,format,c->data_segment,value);dg_text(c,x,120,text);
}
static void dg_append(JxEventContext *c,JxFarPointer frame,JxFarPointer source) {
    JxFarPointer text=dg_at(frame,104);(void)JX_CALL(c,0x1000,0x23f4,text.offset,text.segment,source.offset,source.segment);
}
static void dg_label(JxEventContext *c,JxFarPointer frame,uint16_t off,uint16_t index) {
    uint16_t at=(uint16_t)(off+index*4u);dg_append(c,frame,(JxFarPointer){dg_get(c,frame,at),dg_get(c,frame,(uint16_t)(at+2))});
}
static void dg_literal(JxEventContext *c,JxFarPointer frame,uint16_t text) {dg_append(c,frame,(JxFarPointer){text,c->data_segment});}
static void dg_dice(JxEventContext *c,JxFarPointer frame) {
    for(uint16_t i=0;i<3;i++) {
        uint16_t value=(uint16_t)JX_CALL0(c,0x1000,0x32b2);
        dg_set(c,frame,(uint16_t)(234+2*i),(uint16_t)(jx_signed_word(value)%6));
    }
}
/* 1F2A:B6A0. Private248-byte compiler frame, preserving live aliasable
 * labels/message/file/dice/balances. Finite, nearest double-to-float domain
 * matches the separately verified legacy software-FP conversion sites. */
uint16_t jx_dice_game(JxEventContext *c,uint16_t unit,uint16_t bankroll,uint16_t opponent_portrait,JxFarPointer frame) {
    static const uint16_t labels[]={0x075c,0x0762,0x0768,0x076e,0x0774,0x077a,0x0780,0x0786,0x078c,0x0792,0x079a,0x07a4,0x07ae,0x07b8,0x07c2,0x07cc,0x07d6,0x07e0,0x07e8,0x07ee,0x07f4,0x07fa};
    for(uint16_t i=0;i<22;i++){dg_set(c,frame,(uint16_t)(16+4*i),labels[i]);dg_set(c,frame,(uint16_t)(18+4*i),c->data_segment);}
    jx_set_word(c,0x2952,2);dg_set(c,frame,242,bankroll);dg_set(c,frame,2,bankroll);
    dg_open(c,frame,0x0800,0x07fd);if(!dg_file(c,frame))return 0xffff;
    dg_seek(c,frame);if(!dg_map(c,91,4))return 0xffff;
    dg_read(c,frame,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)},1,64000);
    dg_read(c,frame,(JxFarPointer){0x2eb4,c->data_segment},3,256);dg_close(c,frame);
    dg_palette(c,0x2eb4);(void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1810,2);dg_image(c,0x1c6c,0,0,320,200,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)});
    dg_image(c,0x1e88,72,0,180,100,dg_buffer(c,12000));
    dg_open(c,frame,0x080e,0x080b);if(!dg_file(c,frame))return 0xffff;
    dg_seek(c,frame);if(!dg_map(c,87,1))return 0xffff;
    dg_read(c,frame,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)},1,1920);dg_close(c,frame);
    dg_dice(c,frame);dg_face(c,dg_get(c,frame,234),147,60);dg_face(c,dg_get(c,frame,236),139,75);dg_face(c,dg_get(c,frame,238),155,75);
    dg_open(c,frame,0x081d,0x081a);dg_seek(c,frame);if(!dg_map(c,88,1))return 0xffff;
    dg_read(c,frame,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)},1,576);dg_close(c,frame);
    uint16_t actor=(uint16_t)(0xd04e + jx_word(c,0x875a)*68u);dg_set(c,frame,6,jx_word(c,(uint16_t)(actor+12)));
    (void)JX_CALL(c,0x1f2a,0x752e,10,0,dg_get(c,frame,6));(void)JX_CALL(c,0x1f2a,0x752e,250,0,opponent_portrait);
    dg_text(c,25,100,(JxFarPointer){0x0828,c->data_segment});dg_text(c,265,100,(JxFarPointer){0x082b,c->data_segment});dg_text(c,25,140,(JxFarPointer){0x082e,c->data_segment});
    (void)JX_CALL(c,0x1832,0x1836,2);(void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);(void)JX_CALL0(c,0x1f2a,0xc24e);(void)JX_CALL(c,0x1832,0x135e,3,2);
    while(jx_signed_word(dg_get(c,frame,242))>=jx_signed_word(unit) && jx_signed_word(dg_get(c,frame,2))>=jx_signed_word(unit)) {
        for(uint16_t i=0;i<122;i++)jx_far_set_byte(c,dg_at(frame,104),i,i<30?32:0);dg_set(c,frame,0,122);
        dg_number(c,frame,dg_get(c,frame,242),0x0833,25);dg_number(c,frame,dg_get(c,frame,2),0x0837,265);
        (void)JX_CALL(c,0x1832,0x135e,dg_page(c,1),dg_page(c,0));dg_image(c,0x1e88,72,0,180,100,dg_buffer(c,30000));
        (void)JX_CALL(c,0x1f2a,0xb21a,0x083b,c->data_segment,0,0,0,0);
        dg_set(c,frame,244,(uint16_t)JX_CALL(c,0x1f2a,0xb522,unit,dg_get(c,frame,242),dg_get(c,frame,2)));
        uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,dg_get(c,frame,96),dg_get(c,frame,98),dg_get(c,frame,100),dg_get(c,frame,102));jx_set_word(c,0x2954,choice);
        (void)JX_CALL(c,0x1503,0x7d,0,145,jx_word(c,0x9332));
        dg_image(c,0x1e88,(uint16_t)(116+(uint16_t)(1-jx_word(c,0x2954))*60u),125,24,24,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)});
        (void)JX_CALL(c,0x1503,0x7d,0,88,jx_word(c,0x9332));
        (void)JX_CALL(c,0x1832,0x1a28,(uint16_t)(116+(uint16_t)(1-jx_word(c,0x2954))*60u),125,24,24,jx_word(c,0x5858),jx_word(c,0x585a),255);
        (void)JX_CALL(c,0x1832,0x1810,dg_page(c,1));
        (void)JX_CALL(c,0x1832,0x1a28,(uint16_t)(116+(uint16_t)(1-jx_word(c,0x2954))*60u),125,24,24,jx_word(c,0x5858),jx_word(c,0x585a),255);
        (void)JX_CALL(c,0x1832,0x1810,dg_page(c,0));dg_dice(c,frame);
        (void)JX_CALL(c,0x1f2a,0xb21a,0x0844,c->data_segment,dg_get(c,frame,234),dg_get(c,frame,236),dg_get(c,frame,238),1);
        JxFarPointer a=dg_at(frame,234),b=dg_at(frame,236),d=dg_at(frame,238);(void)JX_CALL(c,0x1f2a,0xb4ba,a.offset,a.segment,b.offset,b.segment,d.offset,d.segment);
        if(dg_get(c,frame,234)!=dg_get(c,frame,236)) {dg_label(c,frame,16,dg_get(c,frame,234));dg_label(c,frame,16,dg_get(c,frame,236));}
        else if(dg_get(c,frame,236)!=dg_get(c,frame,238)) {dg_literal(c,frame,0x084e);dg_label(c,frame,16,dg_get(c,frame,236));}
        else dg_literal(c,frame,0x0851);
        dg_label(c,frame,16,dg_get(c,frame,238));dg_literal(c,frame,0x0856);
        dg_set(c,frame,0,(uint16_t)(dg_get(c,frame,234)+dg_get(c,frame,236)+dg_get(c,frame,238)));
        dg_label(c,frame,16,(uint16_t)(dg_get(c,frame,0)+2));dg_literal(c,frame,0x0859);
        dg_set(c,frame,0,jx_signed_word(dg_get(c,frame,0))>7?1:0);
        uint16_t won=dg_get(c,frame,0)==jx_word(c,0x2954);dg_set(c,frame,232,won);
        dg_set(c,frame,242,(uint16_t)(dg_get(c,frame,242)+(won?dg_get(c,frame,244):(uint16_t)(0-dg_get(c,frame,244)))));
        dg_set(c,frame,2,(uint16_t)(dg_get(c,frame,2)+(won?(uint16_t)(0-dg_get(c,frame,244)):dg_get(c,frame,244))));
        dg_label(c,frame,96,dg_get(c,frame,0));dg_label(c,frame,88,dg_get(c,frame,232));dg_literal(c,frame,0x085c);
        JxFarPointer message=dg_at(frame,104);(void)JX_CALL(c,0x2f28,0x6572,30,160,2,message.offset,message.segment,1);
        (void)JX_CALL(c,0x1503,0x7d,0,145,jx_word(c,0x9332));dg_image(c,0x1c6c,(uint16_t)(116+(uint16_t)(1-jx_word(c,0x2954))*60u),125,24,24,(JxFarPointer){jx_word(c,0x5858),jx_word(c,0x585a)});
        (void)JX_CALL(c,0x1832,0x135e,dg_page(c,1),dg_page(c,0));
    }
    uint16_t outcome=jx_signed_word(dg_get(c,frame,242))<jx_signed_word(unit)?0:1;dg_set(c,frame,240,outcome);
    uint32_t money=jx_word(c,0x8756)|((uint32_t)jx_word(c,0x8758)<<16),signed_bankroll=(uint32_t)(int32_t)jx_signed_word(bankroll);
    money=outcome?money+signed_bankroll:money-signed_bankroll;jx_set_word(c,0x8756,(uint16_t)money);jx_set_word(c,0x8758,(uint16_t)(money>>16));
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));(void)JX_CALL(c,0x1832,0x1836,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);dg_palette(c,0x61de);(void)JX_CALL0(c,0x1f2a,0x5ae8);(void)JX_CALL0(c,0x1f2a,0xc24e);return dg_get(c,frame,240);
}
