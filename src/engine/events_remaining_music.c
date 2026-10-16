#include "event_engine.h"
static void remaining_music(JxEventContext *c,uint16_t track){
    jx_set_word(c,0x889a,track);
    if((int16_t)jx_word(c,0x31b4)>0 && (int16_t)jx_word(c,0x050c)>0){
        uint16_t seconds=jx_far_word(c,jx_word(c,0x23a0),(uint16_t)(0x510+2*track));
        (void)JX_CALL(c,0x1832,0x224c,(uint8_t)(jx_ds(c)[0x932c]+track),2,seconds);
    }
}
void jx_callback_1f2a_f20e(JxEventContext *c){
    if(jx_ds(c)[0xbc80])return;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5b46),jx_word(c,0x5b48),1);
    jx_set_word(c,0x875a,0);jx_set_word(c,0x875c,1);jx_set_word(c,0x875e,0xffff);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);
    remaining_music(c,18);
    (void)JX_CALL0(c,0x2f28,0x5580);
    jx_ds(c)[0xbc80]=1;
}
void jx_callback_1f2a_f822(JxEventContext *c){
    if(!jx_ds(c)[0x34c0])return;
    jx_set_word(c,0x8760,11);jx_set_word(c,0x8762,7);jx_set_word(c,0x8764,1);
    jx_set_word(c,0x8766,11);jx_set_word(c,0x8768,7);
    jx_set_word(c,0x876c,11);jx_set_word(c,0x876e,7);
    jx_set_word(c,0x9334,90);jx_set_word(c,0x8894,3);
    (void)JX_CALL0(c,0x1f2a,0x55f2);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    remaining_music(c,5);
}
void jx_callback_1f2a_f950(JxEventContext *c){
    if(jx_word(c,0x875c)!=1)return;
    if(jx_ds(c)[0x7052]){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5d2e),jx_word(c,0x5d30),1);
        (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
        jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x8760,22);jx_set_word(c,0x8762,28);jx_set_word(c,0x8764,0);
        jx_set_word(c,0x9334,81);jx_ds(c)[0x34ac]=0;
        (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    }else{
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5c0a),jx_word(c,0x5c0c),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x08e3,c->data_segment,2);
    }
}
void jx_callback_2f28_149a(JxEventContext *c){
    if(!(uint16_t)JX_CALL(c,0x1f2a,0x8ca8,31))return;
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x8760,2);jx_set_word(c,0x8762,9);jx_set_word(c,0x8764,3);jx_set_word(c,0x9334,147);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_ds(c)[0x34f7]=0;remaining_music(c,10);
}
void jx_callback_2f28_2eb4(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xce14])return;
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5afa),jx_word(c,0x5afc),1);
    jx_set_word(c,0x875c,1);(void)JX_CALL0(c,0x1f2a,0xc6a8);
    jx_ds(c)[(uint16_t)(0x7dca+npc)]=0;jx_ds(c)[0xcdfc]=5;
    (void)JX_CALL0(c,0x2f28,0x5580);remaining_music(c,3);
}
void jx_callback_2f28_30d0(JxEventContext *c){
    if(!jx_ds(c)[0xce92])return;
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x8760,19);jx_set_word(c,0x8762,124);jx_set_word(c,0x8764,0);
    jx_set_word(c,0x876c,20);jx_set_word(c,0x876e,124);jx_set_word(c,0x8770,0);
    jx_set_word(c,0x9334,213);remaining_music(c,6);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    if(jx_ds(c)[0xce92]!=5){
        (void)JX_CALL(c,0x2f28,0x648e,20,jx_word(c,0x5b4e),jx_word(c,0x5b50),1);
        jx_ds(c)[0x7f64]=0;(void)JX_CALL0(c,0x2f28,0x5580);
    }
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5b4a),jx_word(c,0x5b4c),1);
    jx_ds(c)[0xce92]=5;
}
void jx_callback_2f28_1734(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x7a1c,npc);
    jx_ds(c)[(uint16_t)(0x77b2+npc)]=0;jx_ds(c)[0x34fc]=1;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5aaa),jx_word(c,0x5aac),1);
    remaining_music(c,16);
}
void jx_callback_2f28_17d0(JxEventContext *c){
    if(jx_ds(c)[0xc81b]!=5)return;
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x8760,16);jx_set_word(c,0x8762,72);jx_set_word(c,0x8764,1);
    jx_set_word(c,0x8766,16);jx_set_word(c,0x8768,74);jx_set_word(c,0x876a,1);
    jx_set_word(c,0x9334,160);jx_set_word(c,0x875a,0);jx_set_word(c,0x875c,4);jx_set_word(c,0x8894,5);
    (void)JX_CALL0(c,0x1f2a,0x55f2);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    remaining_music(c,13);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x59da),jx_word(c,0x59dc),1);
}
void jx_callback_2f28_18ec(JxEventContext *c){
    jx_ds(c)[0x780f]=0;(void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5ba6),jx_word(c,0x5ba8),1);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5ba2),jx_word(c,0x5ba4),1);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5baa),jx_word(c,0x5bac),1);
    jx_ds(c)[0xc81b]=5;
}
void jx_callback_2f28_0014(JxEventContext *c){
    if(jx_ds(c)[0xc14d])return;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x59ea),jx_word(c,0x59ec),1);
    jx_ds(c)[0xc14d]=1;remaining_music(c,8);
}
void jx_callback_2f28_047c(JxEventContext *c){
    if(jx_ds(c)[0xc23f])return;
    (void)JX_CALL(c,0x1f2a,0x966e,0,30);
    (void)JX_CALL(c,0x2f28,0x648e,4,jx_word(c,0x5ab2),jx_word(c,0x5ab4),1);
    jx_ds(c)[0x34cf]=1;remaining_music(c,9);
}
void jx_callback_2f28_03d0(JxEventContext *c){
    if(jx_ds(c)[0xc23c]!=1)return;
    (void)JX_CALL(c,0x2f28,0x648e,4,jx_word(c,0x5ab6),jx_word(c,0x5ab8),1);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x2dbc,30);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x966e,0,29);
    (void)JX_CALL(c,0x2f28,0x648e,4,jx_word(c,0x5aba),jx_word(c,0x5abc),1);
    jx_ds(c)[0xc23c]=2;
}
void jx_callback_1f2a_f460(JxEventContext *c){
    if(jx_ds(c)[0xbd50] || jx_ds(c)[0xc07b]!=1)return;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5d2a),jx_word(c,0x5d2c),1);
    jx_ds(c)[0xbd50]=1;jx_ds(c)[0x34b8]=0;jx_ds(c)[0x7052]=1;
    for(uint16_t i=0;i<10;++i)jx_ds(c)[0xc004+i]=5;
    jx_set_word(c,0x889a,9);
    /* This original callback checks only the sound setting, not CD presence. */
    if((int16_t)jx_word(c,0x050c)>0){
        uint16_t seconds=jx_far_word(c,jx_word(c,0x23a0),0x0522);
        (void)JX_CALL(c,0x1832,0x224c,(uint8_t)(jx_ds(c)[0x932c]+9),2,seconds);
    }
}
void jx_callback_1f2a_f648(JxEventContext *c){
    if(jx_word(c,0x875e)!=8)return;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5caa),jx_word(c,0x5cac),1);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5cae),jx_word(c,0x5cb0),1);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5cb2),jx_word(c,0x5cb4),1);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x875e,0xffff);
    for(uint16_t i=0;i<3;++i){jx_set_word(c,(uint16_t)(0x8760+6*i),65);jx_set_word(c,(uint16_t)(0x8762+6*i),52);}
    (void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    remaining_music(c,12);
}
void jx_callback_2f28_0c78(JxEventContext *c){
    if(jx_ds(c)[0xc4ef] || !jx_ds(c)[0xc41c])return;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5c16),jx_word(c,0x5c18),1);
    (void)JX_CALL(c,0x2f28,0x7062,0x0958,c->data_segment,3,0);
    jx_set_word(c,0x8760,45);jx_set_word(c,0x8762,90);jx_set_word(c,0x8764,1);
    jx_set_word(c,0x8766,45);jx_set_word(c,0x8768,91);
    jx_set_word(c,0x876c,45);jx_set_word(c,0x876e,92);
    (void)JX_CALL(c,0x1f2a,0x8f28,29,1);
    jx_set_word(c,0x9334,123);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0xd05c,259);jx_set_word(c,0xd0a0,260);jx_ds(c)[0xc4ef]=1;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    remaining_music(c,18);
}
void jx_callback_2f28_0fd6(JxEventContext *c){
    if(!(uint16_t)JX_CALL(c,0x1f2a,0x8ca8,32))return;
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x8760,2);jx_set_word(c,0x8762,9);jx_set_word(c,0x8764,3);jx_set_word(c,0x9334,147);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_ds(c)[0x34f7]=0;remaining_music(c,10);
}
void jx_callback_2f28_1b68(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xcb44])return;
    jx_set_word(c,0x8760,28);jx_set_word(c,0x8762,91);jx_set_word(c,0x8764,0);jx_set_word(c,0x9334,183);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    (void)JX_CALL(c,0x2f28,0x944c,0x097e,c->data_segment,3);
    jx_set_word(c,0x8764,1);jx_set_word(c,0x8770,1);(void)JX_CALL0(c,0x2f28,0x5580);
    remaining_music(c,9);
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5ad2),jx_word(c,0x5ad4),1);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x875e,0xffff);(void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    jx_set_word(c,0x8770,0);(void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5ad6),jx_word(c,0x5ad8),1);
}
void jx_callback_2f28_2072(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a9e),jx_word(c,0x5aa0),1);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_ds(c)[0x7aa5]=0;jx_ds(c)[0x7aa6]=1;(void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    for(uint16_t text=0x5aa2;text<=0x5aaa;text+=4){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,text),jx_word(c,(uint16_t)(text+2)),1);
    }
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0x875a,3);jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x875e,0xffff);
    jx_set_word(c,0x8760,14);jx_set_word(c,0x8762,68);jx_set_word(c,0x8764,0);jx_set_word(c,0x9334,174);
    jx_ds(c)[0x350f]=0;jx_ds(c)[0x7adc]=1;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    remaining_music(c,10);
}
void jx_callback_2f28_1d32(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a5a),jx_word(c,0x5a5c),1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,npc,0x0996,c->data_segment,3);
    jx_ds(c)[0x7942]=0;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5a8a),jx_word(c,0x5a8c),1);
    jx_ds(c)[0x79ce]=1;
}
void jx_callback_2f28_331c(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b52),jx_word(c,0x5b54),1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,35,0,1)==1){
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}return;
    }
    jx_ds(c)[(uint16_t)(0x7f50+npc)]=0;(void)JX_CALL0(c,0x2f28,0x5580);
    int16_t random=(int16_t)JX_CALL0(c,0x1000,0x32b2);
    (void)JX_CALL(c,0x1f2a,0x8cea,(uint16_t)(42+random%16),1);
    jx_set_word(c,0x8760,33);jx_set_word(c,0x8762,82);jx_set_word(c,0x8764,1);
    jx_set_word(c,0x876c,34);jx_set_word(c,0x876e,82);jx_set_word(c,0x8770,1);jx_set_word(c,0x9334,200);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);remaining_music(c,3);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    (void)JX_CALL(c,0x1f2a,0x747c,0);(void)JX_CALL(c,0x1f2a,0x747c,1);
    (void)JX_CALL(c,0x2f28,0x648e,6,jx_word(c,0x5b0e),jx_word(c,0x5b10),0);
}
