#include "event_engine.h"

static JxFarPointer file_pointer(uint32_t value) {
    return (JxFarPointer){(uint16_t)value,(uint16_t)(value>>16)};
}
static void cinematic_frames(JxEventContext *c,JxFarPointer file,unsigned first) {
    for(unsigned frame=first;frame<first+2;frame++) {
        for(uint16_t bank=0;bank<4;bank++)
            (void)JX_CALL(c,0x1503,0x007d,bank,(uint16_t)(0x57+frame*4+bank),jx_word(c,0x9332));
        (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,64000,file.offset,file.segment);
    }
}

/* Overlay 180:0000. Load backdrop/palette and four EMS animation frames,
 * then scroll once per three timer events, finishing on event 2501.
 * The third file is intentionally left open, as in the original. */
JxFlow jx_cinematic_180(JxEventContext *c,JxFarPointer backdrop,JxFarPointer frames,
    JxInputPoll poll,void *opaque) {
    uint32_t handle=JX_CALL(c,0x1000,0x0738,backdrop.offset,backdrop.segment,0x0dfb,c->data_segment);
    if(!handle) return JX_FLOW_RETURNED;
    JxFarPointer file=file_pointer(handle);
    (void)JX_CALL(c,0x1000,0x27fa,file.offset,file.segment,16,0,0);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,64000,file.offset,file.segment);
    (void)JX_CALL(c,0x1000,0x0754,0x2eb4,c->data_segment,3,256,file.offset,file.segment);
    (void)JX_CALL(c,0x1000,0x0636,file.offset,file.segment);
    handle=JX_CALL(c,0x1000,0x0738,frames.offset,frames.segment,0x0dfe,c->data_segment);
    if(!handle) return JX_FLOW_RETURNED;
    file=file_pointer(handle);
    (void)JX_CALL(c,0x1000,0x27fa,file.offset,file.segment,8,0,0);
    cinematic_frames(c,file,0);
    (void)JX_CALL(c,0x1000,0x0636,file.offset,file.segment);
    (void)JX_CALL(c,0x1832,0x1810,0);
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
    handle=JX_CALL(c,0x1000,0x0738,0x0e04,c->data_segment,0x0e01,c->data_segment);
    if(!handle) {
        (void)JX_CALL(c,0x1000,0x01eb,1);
        return JX_FLOW_EXIT_1;
    }
    file=file_pointer(handle);
    (void)JX_CALL(c,0x1000,0x27fa,file.offset,file.segment,16,0,0);
    cinematic_frames(c,file,2);
    (void)JX_CALL(c,0x1832,0x1810,0);
    (void)JX_CALL(c,0x1832,0x1836,0);
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    jx_input_clear(c);
    uint16_t ticks=0,position=40;
    for(;;) {
        JxInputEvent event;
        if(poll(opaque,&event)!=0 || event.type!=8) continue;
        ticks++;
        if(ticks>2500) {
            jx_input_clear(c);
            (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
            for(uint16_t page=0;page<2;page++) {
                (void)JX_CALL(c,0x1832,0x1810,page);
                (void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
            }
            (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
            jx_event_refresh(c);
            jx_event_transition_end(c);
            return JX_FLOW_RETURNED;
        }
        if(ticks%3==0) {
            position++;
            uint16_t scroll=position>960?960:position;
            (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(position%2));
            (void)JX_CALL(c,0x2f28,0x8012,(uint16_t)(position%2));
            (void)JX_CALL(c,0x2f28,0x809c,scroll);
            (void)JX_CALL(c,0x1832,0x1836,(uint16_t)(position%2));
        }
    }
}

/* Overlay 180:03E0. Battle 28 branches into two cinematics or an alternate
 * dialogue path. Fatal flow from the resource loader is never swallowed. */
JxFlow jx_cinematic_event_180(JxEventContext *c,int16_t npc,JxInputPoll poll,void *opaque) {
    jx_event_text(c,npc,0x5aae,1);jx_event_text(c,npc,0x5ab2,1);
    const uint16_t setup[]={0,0,119,0,6,0,120,0,12,0,121,0};
    for(unsigned i=0;i<12;i++) jx_set_word(c,(uint16_t)(0x64e4+i*2),setup[i]);
    (void)JX_CALL(c,0x2f28,0x7062,0x1206,c->data_segment,7,0);
    int battle=jx_signed_word((uint16_t)JX_CALL(c,0x1f2a,0x4934,28,3,0xffff));
    if(battle==1) {
        jx_event_transition_begin(c);
        JxFlow flow=jx_cinematic_180(c,(JxFarPointer){0x121a,c->data_segment},
            (JxFarPointer){0x1212,c->data_segment},poll,opaque);
        if(flow!=JX_FLOW_RETURNED) return flow;
    } else if(battle<0) {
        jx_event_text(c,npc,0x5ab6,1);
        jx_ds(c)[(uint16_t)(0x7adc+npc)]=0;jx_ds(c)[0x7add]=1;
        jx_event_refresh(c);jx_event_text(c,npc,0x5aba,1);
        (void)JX_CALL(c,0x1f2a,0xdb46,1);
        jx_event_text(c,npc,0x5a8e,1);
        (void)JX_CALL(c,0x2f28,0x6f12,0x1222,c->data_segment,500,1);
        jx_ds(c)[0xd04d]=1;
    } else {
        jx_event_transition_begin(c);
        JxFlow flow=jx_cinematic_180(c,(JxFarPointer){0x123a,c->data_segment},
            (JxFarPointer){0x122e,c->data_segment},poll,opaque);
        if(flow!=JX_FLOW_RETURNED) return flow;
        (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
        jx_event_transition_end(c);
        (void)JX_CALL(c,0x1f2a,0x5fd8,4,6);
        (void)JX_CALL(c,0x1f2a,0x155c,6);
        (void)JX_CALL(c,0x2f28,0x3b8e,0x1244,c->data_segment);
    }
    (void)JX_CALL(c,0x1f2a,0x5fd8,35,0);
    (void)JX_CALL(c,0x1f2a,0x5fd8,34,1);
    (void)JX_CALL(c,0x1f2a,0x5fd8,38,2);
    jx_set_word(c,0x875a,0);jx_set_word(c,0x875c,1);jx_set_word(c,0x875e,4);
    jx_event_position(c,0x8760,80,4,1);jx_set_word(c,0x9334,161);
    jx_ds(c)[0x350f]=0;jx_ds(c)[0x7adc]=0;jx_ds(c)[0x7b18]=1;
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    jx_event_music(c,13,2);jx_event_text(c,0,0x5aca,1);
    return JX_FLOW_RETURNED;
}
