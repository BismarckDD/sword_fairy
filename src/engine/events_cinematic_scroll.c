#include "event_engine.h"
static void cinematic_map_four(JxEventContext *c,uint16_t first){
    for(uint16_t slot=0;slot<4;++slot)(void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
static JxFarPointer cinematic_huge_add(JxFarPointer p,int32_t delta){
    uint32_t low=(uint32_t)p.offset+(uint16_t)delta;
    uint16_t high=(uint16_t)((uint32_t)delta>>16)+(uint16_t)(low>>16);
    return (JxFarPointer){(uint16_t)low,(uint16_t)(p.segment+(uint16_t)(high<<12))};
}
void jx_cinematic_backdrop(JxEventContext *c,uint16_t frame){
    cinematic_map_four(c,(uint16_t)(87+4u*frame));
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
}
void jx_cinematic_scroll(JxEventContext *c,uint16_t position){
    int16_t signed_position=(int16_t)position;
    if(signed_position<320 || signed_position>=640){
        (void)JX_CALL(c,0x1000,0x31a4,jx_word(c,0x2eb0),jx_word(c,0x2eb2),255,64000);
    }
    for(uint16_t half=0;half<2;++half){
        cinematic_map_four(c,(uint16_t)(95+4*half));
        for(uint16_t row=0;row<100;++row){
            uint16_t count=signed_position<320?position:signed_position<640?320:(uint16_t)(960-position);
            uint16_t source=(uint16_t)(jx_word(c,0x5858)+640u*row);
            if(signed_position<640)source=(uint16_t)(source+640-position);
            JxFarPointer target={jx_word(c,0x2eb0),jx_word(c,0x2eb2)};
            target=cinematic_huge_add(target,(uint16_t)(320u*(row+100u*half)));
            if(signed_position>=640){
                target=cinematic_huge_add(target,signed_position);
                target=cinematic_huge_add(target,-640);
            }
            (void)JX_CALL(c,0x1000,0x3146,target.offset,target.segment,source,jx_word(c,0x585a),count);
        }
    }
    (void)JX_CALL(c,0x1832,0x1a28,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2),255);
}
