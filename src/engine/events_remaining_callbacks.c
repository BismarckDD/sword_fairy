#include "event_engine.h"

static void quest_wait_for_death(JxEventContext *c){
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
}
void jx_callback_2f28_0a1c(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xc41d]){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5be6),jx_word(c,0x5be8),1);
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5bea),jx_word(c,0x5bec),1);
        jx_ds(c)[0x72e4]=1;jx_ds(c)[0xc41d]=1;
    }
}
void jx_callback_2f28_122e(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a2a),jx_word(c,0x5a2c),1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,18,1,0xffff)==1){quest_wait_for_death(c);return;}
    jx_ds(c)[(uint16_t)(0x760e + npc)]=0;
    (void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x1f2a,0x8cea,30,1);
}
void jx_callback_2f28_12be(JxEventContext *c,uint16_t npc){
    if(jx_ds(c)[(uint16_t)(0xc676+npc)])return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,19,1,0xffff)==1){quest_wait_for_death(c);return;}
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,20,1,0xffff)==1){quest_wait_for_death(c);return;}
    jx_ds(c)[0xc5fe]=5;jx_ds(c)[0xc65a]=5;jx_ds(c)[0xc65b]=5;
    jx_ds(c)[0xc676]=5;jx_ds(c)[0xc677]=5;
    (void)JX_CALL(c,0x1f2a,0x8cea,32,1);
}
/* 2F28:2D9C. Preserve the original signed high-word money comparison and
 * unsigned low word/reputation comparisons. NPC1 deliberately aliases the
 * subsequent fixed CD61 write, so it ends in state5 rather than6. */
void jx_callback_2f28_2d9c(JxEventContext *c,uint16_t npc){
    uint16_t state=(uint16_t)(0xcd60+npc);
    if((int8_t)jx_ds(c)[state]==5){
        jx_ds(c)[state]=6;jx_ds(c)[0xcd61]=5;
        uint16_t low=jx_word(c,0x8756),high=jx_word(c,0x8758);
        if((int16_t)high<0 || (!high && low<300)){
            jx_set_word(c,0x8756,0);jx_set_word(c,0x8758,0);
        }else{
            jx_set_word(c,0x8756,(uint16_t)(low-300));
            jx_set_word(c,0x8758,(uint16_t)(high-(low<300)));
        }
        uint16_t reputation=jx_word(c,0x8752);
        jx_set_word(c,0x8752,reputation>15?(uint16_t)(reputation-15):0);
    }else if((int8_t)jx_ds(c)[state]!=6)jx_ds(c)[0xc96a]=5;
}
/* 1F2A:DE00 / EC30 / EE48 / F912. Conditional world/NPC callbacks.
 * Calls stay ordered and final flag writes occur after callback mutations. */
void jx_callback_1f2a_de00(JxEventContext *c){
    if(!jx_ds(c)[0x3466]){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5cce),jx_word(c,0x5cd0),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x08cd,c->data_segment,3);
    }
}
void jx_callback_1f2a_ec30(JxEventContext *c){
    if(jx_ds(c)[0xba62]&&!jx_ds(c)[0xba43]){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5aee),jx_word(c,0x5af0),1);
        jx_ds(c)[0xba43]=1;
    }
}
void jx_callback_1f2a_ee48(JxEventContext *c){
    if(jx_ds(c)[0xbaa0]==5){
        (void)JX_CALL(c,0x1f2a,0x8cea,51,1);
        (void)JX_CALL(c,0x1f2a,0x8cea,53,1);jx_ds(c)[0xbaa0]=6;
    }
}
void jx_callback_1f2a_f912(JxEventContext *c){
    if(jx_ds(c)[0xbd34]!=6){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5bfe),jx_word(c,0x5c00),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x08df,c->data_segment,2);
    }
}
void jx_callback_1f2a_fece(JxEventContext *c){
    if(jx_ds(c)[0xc07b]==1){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5cfa),jx_word(c,0x5cfc),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x0921,c->data_segment,2);
    }
}
void jx_callback_2f28_013a(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a2a),jx_word(c,0x5a2c),1);
    jx_ds(c)[(uint16_t)(0xc18a+npc)]=5;
    (void)JX_CALL(c,0x1f2a,0x8f28,28,1);
    jx_ds(c)[0x71d6]=1;jx_ds(c)[0xc204]=5;
}
void jx_callback_2f28_1942(JxEventContext *c){
    if(!jx_ds(c)[0xc855]){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5b96),jx_word(c,0x5b98),1);
        jx_ds(c)[0xc855]=1;jx_ds(c)[0x77ef]=1;jx_ds(c)[0x77f0]=0;jx_ds(c)[0x34fc]=1;
    }
}
void jx_callback_2f28_064e(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5ba6),jx_word(c,0x5ba8),1);
    (void)JX_CALL(c,0x1f2a,0x8cea,15,1);
    jx_ds(c)[0x72e4]=0;jx_ds(c)[0x739c]=0;
    jx_ds(c)[0x73d4]=1;jx_ds(c)[0x73d6]=1;jx_ds(c)[0x73d7]=1;jx_ds(c)[0xc401]=5;
    (void)JX_CALL0(c,0x2f28,0x5580);
}
void jx_callback_2f28_308a(JxEventContext *c){
    if((int8_t)jx_ds(c)[0xce90]<=4 && jx_ds(c)[0xcf3f]){
        (void)JX_CALL(c,0x2f28,0x648e,5,jx_word(c,0x5b6e),jx_word(c,0x5b70),1);
        jx_ds(c)[0xce90]=5;jx_ds(c)[0xce91]=5;
    }
}
void jx_callback_1f2a_e69a(JxEventContext *c,uint16_t npc){
    uint16_t frame=(uint16_t)(0x88d8+86u*npc),present=(uint16_t)(0x68ef+npc);
    if(!jx_word(c,frame) && jx_ds(c)[present])(void)JX_CALL(c,0x17d2,0x00d8,npc);
}
void jx_callback_1f2a_f72e(JxEventContext *c){
    if(jx_ds(c)[0x6df9]){
        (void)JX_CALL(c,0x1f2a,0xcf6a,1,11,40);
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5cb6),jx_word(c,0x5cb8),1);
        jx_ds(c)[0x6df9]=0;
    }
}
void jx_callback_2f28_11c4(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a36),jx_word(c,0x5a38),1);
    (void)JX_CALL(c,0x1f2a,0x8f28,30,1);(void)JX_CALL(c,0x1f2a,0x8cea,32,1);
    jx_ds(c)[(uint16_t)(0xc61c+npc)]=6;
}
void jx_callback_2f28_01a2(JxEventContext *c,uint16_t npc){
    if((int8_t)jx_ds(c)[0xc205]<=4){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a12),jx_word(c,0x5a14),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x0929,c->data_segment,2);
    }
}
void jx_callback_2f28_06e4(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xc41c]){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5bc6),jx_word(c,0x5bc8),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x092e,c->data_segment,3);
    }
}
void jx_callback_2f28_0aa0(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xc476]){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b9e),jx_word(c,0x5ba0),1);
        jx_ds(c)[0x7434]=0;(void)JX_CALL0(c,0x2f28,0x5580);
        jx_ds(c)[0x34f3]=0;jx_ds(c)[0x34f4]=0;jx_ds(c)[0x34f5]=1;jx_ds(c)[0x34f6]=1;jx_ds(c)[0xc476]=1;
    }
}

void jx_callback_1f2a_eaac(JxEventContext *c){
    if(!jx_ds(c)[0xb9eb] && jx_ds(c)[0xb9ce]==5 && jx_ds(c)[0x69a2]){
        (void)JX_CALL(c,0x2f28,0x648e,0xffff,jx_word(c,0x601e),jx_word(c,0x6020),1);
        jx_ds(c)[0xb9eb]=1;
    }
}

void jx_callback_1f2a_f108(JxEventContext *c){
    if(jx_ds(c)[0x6c36]){
        (void)JX_CALL(c,0x1f2a,0x5fd8,19,6);
        (void)JX_CALL(c,0x1f2a,0x155c,6);
        (void)JX_CALL(c,0x1f2a,0xdb46,0);
        jx_ds(c)[0x6c36]=0;jx_ds(c)[0xbc62]=1;
        (void)JX_CALL0(c,0x2f28,0x5580);
    }
}
void jx_callback_1f2a_f150(JxEventContext *c){
    if(!jx_ds(c)[0x6c36]){
        jx_ds(c)[0x6c37]=1;
        (void)JX_CALL(c,0x1f2a,0x5fd8,19,6);
        (void)JX_CALL(c,0x1f2a,0x155c,6);
        (void)JX_CALL(c,0x1f2a,0xdb46,1);
        jx_ds(c)[0x6c37]=0;jx_ds(c)[0x6c36]=1;jx_set_word(c,0x88d8,0);
        (void)JX_CALL0(c,0x2f28,0x5580);
    }
}
void jx_callback_1f2a_f3a4(JxEventContext *c){
    int8_t state=(int8_t)jx_ds(c)[0xbcf9];
    if(state>=5 && state!=7){
        (void)JX_CALL(c,0x2f28,0x648e,1,jx_word(c,0x5bc2),jx_word(c,0x5bc4),1);
        (void)JX_CALL(c,0x1f2a,0x8cea,5,1);
        jx_ds(c)[0xbcf8]=5;jx_ds(c)[0xbcf9]=7;
    }
}

void jx_callback_2f28_09b4(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xc41c]){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5c0e),jx_word(c,0x5c10),1);
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5c12),jx_word(c,0x5c14),1);
        (void)JX_CALL(c,0x1f2a,0x8cea,29,1);
        jx_ds(c)[0x73be]=1;jx_ds(c)[0xc41c]=1;jx_ds(c)[0xc3fe]=1;jx_ds(c)[0xc3ff]=1;jx_ds(c)[0xc477]=1;
    }
}
void jx_callback_2f28_0334(JxEventContext *c){
    if(jx_ds(c)[0xc23c]==2){
        (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
        (void)JX_CALL(c,0x1f2a,0x2dbc,30);
        (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
        (void)JX_CALL(c,0x1f2a,0x966e,1,28);
        (void)JX_CALL(c,0x2f28,0x648e,4,jx_word(c,0x5abe),jx_word(c,0x5ac0),1);
        jx_ds(c)[0xc23c]=3;
    }
}
void jx_callback_2f28_07c6(JxEventContext *c){
    if(jx_ds(c)[0xc477] && !jx_ds(c)[0xc3fe]){
        jx_ds(c)[0x73c1]=1;
        (void)JX_CALL(c,0x1f2a,0xcf6a,11,jx_word(c,0x8760),(uint16_t)(jx_word(c,0x8762)-1));
        (void)JX_CALL(c,0x2f28,0x944c,0x0932,c->data_segment,2);
        (void)JX_CALL(c,0x1f2a,0xcd1c,11,0x0934,c->data_segment,3);
        jx_set_word(c,0x875c,1);
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5b7a),jx_word(c,0x5b7c),1);
        jx_set_word(c,0xd0b4,(uint16_t)(jx_word(c,0xd0b4)+2));
        (void)JX_CALL(c,0x1f2a,0x747c,1);
        jx_ds(c)[0x73c1]=0;
        (void)JX_CALL0(c,0x1f2a,0xc6a8);
        (void)JX_CALL0(c,0x2f28,0x5580);
        jx_ds(c)[0xc3fe]=1;
    }
}

void jx_callback_1f2a_ea1a(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xb9ea]){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x6006),jx_word(c,0x6008),1);
        (void)JX_CALL(c,0x1f2a,0xcd1c,npc,0x08d1,c->data_segment,3);
        uint16_t object=(uint16_t)(0x889c+86u*npc);
        jx_set_word(c,(uint16_t)(object+0x40),4);
        jx_set_word(c,(uint16_t)(object+0x42),21);
        jx_set_word(c,(uint16_t)(object+0x44),0);
        (void)JX_CALL0(c,0x2f28,0x5580);
        jx_ds(c)[0x346b]=0;jx_ds(c)[0xb9ea]=1;
    }
}


static void quest_remove_guard(JxEventContext *c,uint16_t npc,uint16_t presence){
    if(!jx_ds(c)[0xc567])return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,16,0,1)==1){quest_wait_for_death(c);return;}
    jx_ds(c)[(uint16_t)(presence+npc)]=0;
    (void)JX_CALL0(c,0x2f28,0x5580);
}
void jx_callback_2f28_0de0(JxEventContext *c,uint16_t npc){quest_remove_guard(c,npc,0x74c4);}
void jx_callback_2f28_0e54(JxEventContext *c,uint16_t npc){quest_remove_guard(c,npc,0x74e2);}
void jx_callback_2f28_15f0(JxEventContext *c,uint16_t npc){
    uint16_t next=(uint16_t)(npc+1);
    jx_ds(c)[(uint16_t)(0x76c2+next)]=1;
    (void)JX_CALL(c,0x1f2a,0xdb46,next);
    (void)JX_CALL(c,0x1f2a,0x5fd8,6,6);
    (void)JX_CALL(c,0x1f2a,0x155c,6);
    (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x0971,c->data_segment,0);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    quest_wait_for_death(c);
}

void jx_callback_2f28_2f44(JxEventContext *c){
    if(jx_ds(c)[0x7dcf]){
        (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5b02),jx_word(c,0x5b04),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x0a09,c->data_segment,3);
    }
}
void jx_callback_2f28_304c(JxEventContext *c){
    if(jx_ds(c)[0xce90]!=5){
        (void)JX_CALL(c,0x2f28,0x648e,5,jx_word(c,0x5b32),jx_word(c,0x5b34),1);
        (void)JX_CALL(c,0x2f28,0x944c,0x0a0d,c->data_segment,3);
    }
}
void jx_callback_2f28_141a(JxEventContext *c,uint16_t npc){
    if((int8_t)jx_ds(c)[0xc6b2]>4)return;
    uint16_t choice=(uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x096e,c->data_segment,0x096b,c->data_segment);
    if(choice){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a4a),jx_word(c,0x5a4c),1);
    }else{
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a4e),jx_word(c,0x5a50),1);
        jx_set_word(c,0x8756,0);jx_set_word(c,0x8758,0);
    }
    jx_ds(c)[(uint16_t)(0xc6b2+npc)]=5;
}

void jx_callback_2f28_238a(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xcc6f]){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b26),jx_word(c,0x5b28),1);
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b2a),jx_word(c,0x5b2c),1);
        jx_ds(c)[0xcc6f]=1;
    }
}
void jx_callback_2f28_23d2(JxEventContext *c){
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    jx_set_word(c,0xd05c,225);jx_set_word(c,0xd0a0,223);
    (void)JX_CALL(c,0x1f2a,0x8f28,35,1);
    (void)JX_CALL0(c,0x1f2a,0xc6a8);
    (void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    jx_ds(c)[0xcc53]=5;
    for(uint16_t i=0;i<10;++i)jx_ds(c)[0xcc34+i]=5;
    jx_ds(c)[0x7c80]=0;jx_ds(c)[0x7c81]=0;
}
void jx_callback_2f28_2482(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b1a),jx_word(c,0x5b1c),1);
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b1e),jx_word(c,0x5b20),1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,npc,0x09cd,c->data_segment,3);
    jx_ds(c)[(uint16_t)(0x7c62+npc)]=0;jx_ds(c)[0x7c10]=1;
    (void)JX_CALL0(c,0x2f28,0x5580);
}
void jx_callback_2f28_24f6(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b0e),jx_word(c,0x5b10),1);
    (void)JX_CALL(c,0x2f28,0x944c,jx_word(c,0x8764)==2?0x09d4:0x09d8,c->data_segment,3);
}
void jx_callback_1f2a_fa0e(JxEventContext *c,uint16_t npc){
    uint16_t state=(uint16_t)(0xbeba+npc);
    if((int8_t)jx_ds(c)[state]>4)return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x08ec,c->data_segment,0x08e7,c->data_segment))return;
    uint16_t low=jx_word(c,0x8756),high=jx_word(c,0x8758);
    if((int16_t)high<0 || (!high && low<100)){
        (void)JX_CALL(c,0x2f28,0x648e,npc,0x08ef,c->data_segment,0);return;
    }
    uint32_t money=((uint32_t)high<<16)|low;money-=100;
    jx_set_word(c,0x8756,(uint16_t)money);jx_set_word(c,0x8758,(uint16_t)(money>>16));
    jx_ds(c)[state]=5;
    (void)JX_CALL(c,0x1f2a,0x8cea,58,1);
}
void jx_callback_2f28_1588(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a66),jx_word(c,0x5a68),1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,17,1,0xffff)==1){quest_wait_for_death(c);return;}
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a6a),jx_word(c,0x5a6c),1);
    jx_ds(c)[0x76c2]=0;(void)JX_CALL0(c,0x2f28,0x5580);
}
void jx_callback_2f28_0526(JxEventContext *c,uint16_t npc){
    if(!jx_ds(c)[0xc567])return;
    uint16_t sprite=jx_word(c,(uint16_t)(0x88aa+86u*npc));
    uint16_t encounter=sprite==0x0108?23:24;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,encounter,0,1)==1){quest_wait_for_death(c);return;}
    uint16_t presence=(uint16_t)(0x665a+30u*jx_word(c,0x9334)+npc);
    jx_ds(c)[presence]=0;(void)JX_CALL0(c,0x2f28,0x5580);
}
void jx_callback_2f28_086a(JxEventContext *c,uint16_t npc){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b86),jx_word(c,0x5b88),1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x8ca8,21)){
        (void)JX_CALL(c,0x1f2a,0x8f28,21,1);
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5b8e),jx_word(c,0x5b90),1);
        (void)JX_CALL(c,0x1f2a,0xcd1c,npc,0x0936,c->data_segment,3);
        jx_ds(c)[(uint16_t)(0x73b6+npc)]=0;
        jx_ds(c)[0x742f]=0;jx_ds(c)[0x7430]=0;
        jx_ds(c)[0x7431]=1;jx_ds(c)[0x7432]=1;jx_ds(c)[0x7433]=1;jx_ds(c)[0x7434]=1;
        uint16_t reputation=jx_word(c,0x8752);
        jx_set_word(c,0x8752,reputation<20?0:(uint16_t)(reputation-20));
    }else{
        (void)JX_CALL(c,0x2f28,0x944c,0x0940,c->data_segment,3);
    }
}
void jx_callback_2f28_1144(JxEventContext *c,uint16_t npc){
    uint16_t state=(uint16_t)(0xc61c+npc);
    if(jx_ds(c)[state])return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x0968,c->data_segment,0x0963,c->data_segment)){
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a02),jx_word(c,0x5a04),1);
        jx_ds(c)[state]=5;
    }else{
        (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,0x5a06),jx_word(c,0x5a08),1);
        jx_ds(c)[0x760e]=1;
    }
}
void jx_callback_2f28_1356(JxEventContext *c,uint16_t npc){
    if(jx_ds(c)[(uint16_t)(0xc676+npc)])return;
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,20,1,0xffff)==1){quest_wait_for_death(c);return;}
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,19,1,0xffff)==1){quest_wait_for_death(c);return;}
    jx_ds(c)[0xc5fe]=5;jx_ds(c)[0xc65a]=5;jx_ds(c)[0xc65b]=5;
    jx_ds(c)[0xc676]=5;jx_ds(c)[0xc677]=5;
    (void)JX_CALL(c,0x1f2a,0x8cea,32,1);
}
void jx_callback_1f2a_fbe0(JxEventContext *c){
    if(!jx_ds(c)[0x6f24])return;
    (void)JX_CALL(c,0x2f28,0x648e,2,jx_word(c,0x5e12),jx_word(c,0x5e14),1);
    jx_set_word(c,0x88dc,jx_word(c,0x8760));jx_set_word(c,0x88de,(uint16_t)(jx_word(c,0x8762)+1));jx_set_word(c,0x88e0,0);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,0x5c82),jx_word(c,0x5c84),1);
    jx_ds(c)[0x6f24]=0;jx_ds(c)[0x6f7e]=1;(void)JX_CALL0(c,0x2f28,0x5580);
    jx_set_word(c,0x8932,jx_word(c,0x8760));jx_set_word(c,0x8934,(uint16_t)(jx_word(c,0x8762)+1));jx_set_word(c,0x8936,0);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL0(c,0x2f28,0x5580);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x2f28,0x648e,1,jx_word(c,0x5c86),jx_word(c,0x5c88),1);
    jx_ds(c)[0x6f25]=0;(void)JX_CALL(c,0x1f2a,0x8cea,26,1);(void)JX_CALL0(c,0x2f28,0x5580);
}
static int quest_can_pay(JxEventContext *c,uint16_t amount){
    int16_t high=(int16_t)jx_word(c,0x8758);
    return high>0 || (high==0 && jx_word(c,0x8756)>=amount);
}
void jx_callback_1f2a_fd28(JxEventContext *c,uint16_t npc){
    if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x0907,c->data_segment,0x0902,c->data_segment))return;
    if(!quest_can_pay(c,20)){
        (void)JX_CALL(c,0x2f28,0x648e,npc,0x090a,c->data_segment,0);return;
    }
    uint32_t money=((uint32_t)jx_word(c,0x8758)<<16)|jx_word(c,0x8756);money-=20;
    jx_set_word(c,0x8756,(uint16_t)money);jx_set_word(c,0x8758,(uint16_t)(money>>16));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875a));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875c));
    jx_set_word(c,0x8760,27);jx_set_word(c,0x8762,6);jx_set_word(c,0x8764,1);jx_set_word(c,0x9334,80);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    if(jx_word(c,0x875c)==1)jx_set_word(c,0x8752,(uint16_t)(jx_word(c,0x8752)+5));
}
void jx_callback_1f2a_fe54(JxEventContext *c,uint16_t npc){
    if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x091e,c->data_segment,0x0919,c->data_segment))return;
    if(quest_can_pay(c,50)){
        (void)JX_CALL(c,0x1f2a,0xb6a0,5,50,jx_word(c,(uint16_t)(0x88a8+86u*npc)));
    }
}
void jx_callback_2f28_1e5e(JxEventContext *c,uint16_t npc){
    if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x09a8,c->data_segment,0x09a3,c->data_segment))return;
    if(!quest_can_pay(c,100)){
        (void)JX_CALL(c,0x2f28,0x648e,npc,0x09ab,c->data_segment,0);return;
    }
    uint32_t money=((uint32_t)jx_word(c,0x8758)<<16)|jx_word(c,0x8756);money-=100;
    jx_set_word(c,0x8756,(uint16_t)money);jx_set_word(c,0x8758,(uint16_t)(money>>16));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875a));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875c));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875e));
    jx_set_word(c,0x8760,20);jx_set_word(c,0x8762,11);jx_set_word(c,0x8764,2);jx_set_word(c,0x9334,164);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_set_word(c,0x8752,(uint16_t)(jx_word(c,0x8752)+1));
}
void jx_callback_2f28_228a(JxEventContext *c){
    if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,100,100,0x09bf,c->data_segment,0x09ba,c->data_segment))return;
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875a));
    (void)JX_CALL(c,0x1f2a,0x747c,jx_word(c,0x875c));
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1f2a,0x2dbc,18);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
}
void jx_callback_1f2a_dd20(JxEventContext *c){
    if(jx_ds(c)[0x3469])return;
    uint16_t text=(int8_t)jx_ds(c)[0xb68a]>4?0x604e:0x6052;
    (void)JX_CALL(c,0x2f28,0x648e,0,jx_word(c,text),jx_word(c,(uint16_t)(text+2)),1);
    (void)JX_CALL(c,0x2f28,0x944c,0x08ca,c->data_segment,4);
}
