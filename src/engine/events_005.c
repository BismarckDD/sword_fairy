#include "event_engine.h"

static void capture_screen(JxEventContext *c) {
    for(uint16_t bank=0;bank<4;bank++)
        (void)JX_CALL(c,0x1503,0x007d,bank,(uint16_t)(0x57+bank),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1e88,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
}
static void wave_screen(JxEventContext *c) {
    capture_screen(c);
    (void)JX_CALL(c,0x17d2,0x01e0,jx_word(c,0x5858),jx_word(c,0x585a),
        jx_word(c,0x2eb0),jx_word(c,0x2eb2),4,10);
}

/* Overlay 005:0000. Opening sequence; original takes no arguments.
 * The common event adapter's npc argument is intentionally unused. */
void jx_event_005_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+22)&255),2,
        jx_far_word(c,jx_word(c,0x23ae),0x053c));
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,192,0);
    (void)JX_CALL(c,0x1f2a,0x06f2,16,0,1);
    (void)JX_CALL(c,0x1f2a,0x0718,8);
    jx_set_word(c,0x08c8,1);
    (void)JX_CALL(c,0x1f2a,0x2dbc,180);
    jx_event_text(c,-1,0x606a,3);
    (void)JX_CALL(c,0x1f2a,0x2dbc,36);
    for(uint16_t text=0x606e;text<=0x607e;text+=4) jx_event_text(c,-1,text,3);
    (void)JX_CALL(c,0x1f2a,0x2dbc,20);
    jx_event_text(c,-1,0x59da,3);jx_set_word(c,0x08c8,0);
    jx_set_word(c,0x889a,3);
    /* This entry starts music unconditionally, unlike the guarded helper. */
    (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+jx_word(c,0x889a))&255),2,
        jx_far_word(c,jx_word(c,0x23ae),(uint16_t)(0x510+2*jx_word(c,0x889a))));
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,192,0);
    (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
    (void)JX_CALL(c,0x1f2a,0x0718,176);
    jx_set_word(c,0x8760,46);jx_set_word(c,0x8762,84);
    (void)JX_CALL0(c,0x1502,0x000a);jx_event_refresh(c);
    capture_screen(c);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    (void)JX_CALL0(c,0x150e,0x000a);
    int effect=jx_signed_word((uint16_t)JX_CALL0(c,0x1000,0x32b2))%8;
    uint16_t segment=0x17d2,offset;
    switch(effect) {
        case 0:offset=0x01e6;break;
        case 1:segment=0x2f28;offset=0x753a;break;
        case 2:case 4:offset=0x01ec;break;
        case 3:offset=0x01f2;break;
        case 5:offset=0x01f8;break;
        case 6:offset=0x01fe;break;
        default:offset=0x0204;break;
    }
    (void)JX_CALL(c,segment,offset,jx_word(c,0x5858),jx_word(c,0x585a));
    (void)JX_CALL(c,0x1f2a,0x2dbc,12);jx_event_text(c,-1,0x59de,1);
    wave_screen(c);jx_event_transition_begin(c);
    (void)JX_CALL0(c,0x1502,0x000a);
    jx_event_position(c,0x8760,39,12,0);
    jx_ds(c)[0x667f]=1;jx_ds(c)[0x6678]=0;jx_set_word(c,0x9334,1);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_event_refresh(c);(void)JX_CALL0(c,0x150e,0x000a);
    (void)JX_CALL0(c,0x2f28,0x7c2e);jx_event_text(c,7,0x59e2,1);
    wave_screen(c);
    (void)JX_CALL(c,0x2f28,0xa58a,0x61de,c->data_segment,0,256);
    jx_event_transition_begin(c);
    uint32_t file=JX_CALL(c,0x1000,0x0738,0x0e96,c->data_segment,0x0e93,c->data_segment);
    /* Original does not check this fopen result before fread/fclose. */
    (void)JX_CALL(c,0x1000,0x0754,0x61de,c->data_segment,3,256,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
    jx_event_position(c,0x8760,46,84,1);
    jx_ds(c)[0x667f]=0;jx_ds(c)[0x6678]=1;jx_set_word(c,0x9334,0);jx_ds(c)[0xb6a3]=1;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_ds(c)[0x6678]=1;jx_event_text(c,0,0x59e6,1);jx_event_literal(c,0,0x0ea1,0);
    jx_set_word(c,0x5120,1);jx_set_word(c,0x5128,1);jx_set_word(c,0x5126,0);
    jx_set_word(c,0x5122,0x0eac);jx_set_word(c,0x5124,c->data_segment);
    jx_set_word(c,0x512a,0x0ebb);jx_set_word(c,0x512c,c->data_segment);
    (void)JX_CALL0(c,0x2f28,0x95aa);(void)JX_CALL(c,0x1f2a,0x2dbc,8);
    jx_event_literal(c,0,0x0ecf,0);jx_event_text(c,0,0x59ea,1);
    jx_ds(c)[0x665a]=0;jx_set_word(c,0x875c,14);
    (void)JX_CALL(c,0x1f2a,0xc264,31,jx_word(c,(uint16_t)(0xd04e + 68*jx_word(c,0x875c)+14)));
    jx_event_refresh(c);(void)JX_CALL(c,0x2f28,0x944c,0x0ee4,c->data_segment,1);
}

/* Overlay 005:0594. Story continuation after the signed NPC state reaches 5. */
void jx_event_005_0594(JxEventContext *c,int16_t npc) {
    if(jx_signed_byte(jx_ds(c)[(uint16_t)(0xb686+npc)])<5) return;
    jx_event_literal(c,0,0x0e5e,0);
    (void)JX_CALL(c,0x2f28,0x6d18,0x0e75,c->data_segment);
    (void)JX_CALL0(c,0x1f2a,0x7272);
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);jx_event_refresh(c);
    (void)JX_CALL(c,0x150f,0x0006,0x61de,c->data_segment,0,256);
    jx_event_grant(c,18,1);jx_event_text(c,0,0x5a2e,1);jx_set_word(c,0x889a,0xffff);
    if(jx_signed_word(jx_word(c,0x31b4))>0 && jx_signed_word(jx_word(c,0x050c))>0)
        (void)JX_CALL0(c,0x1832,0x22f2);
    jx_set_word(c,0x64e4,3);jx_set_word(c,0x64e6,1);jx_set_word(c,0x64e8,77);
    (void)JX_CALL(c,0x2f28,0x7062,0x0e7f,c->data_segment,7,0);
    jx_ds(c)[0x665b]=1;jx_event_refresh(c);
    (void)JX_CALL(c,0x2f28,0x944c,0x0e8a,c->data_segment,2);
    jx_event_text(c,1,0x5a32,1);
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,1,0,0xffff)==1) {
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
        return;
    }
    jx_event_text(c,1,0x5a36,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,1,0x0e8c,c->data_segment,2);
    jx_ds(c)[0x665b]=0;jx_ds(c)[0x3469]=1;jx_event_transition_begin(c);
    jx_event_music(c,5,0);
    for(unsigned i=0;i<4;i++) jx_ds(c)[0x665c+i]=0;
    jx_event_refresh(c);jx_event_transition_end(c);
}
