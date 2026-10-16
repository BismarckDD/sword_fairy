#include "event_engine.h"
#include <assert.h>

static JxFarPointer wave_pointer(JxFarPointer p,unsigned displacement) {
    uint32_t sum=(uint32_t)p.offset+displacement;
    return (JxFarPointer){(uint16_t)sum,(uint16_t)(p.segment+(sum>>16)*4096u)};
}

/* Overlay 009:0498. Shift eight repeating scanlines back and forth over
 * 192 rows. The final full-height source frame follows limit+1 wave frames.
 * A zero divisor faults in the original; it is a caller precondition here.
 * A limit of 65535 never finishes, just as in the original word counter. */
void jx_transition_wave(JxEventContext *c,JxFarPointer source,JxFarPointer work,
    uint16_t tick_divisor,uint16_t frame_limit,JxInputPoll poll,void *input_opaque) {
    uint16_t shifts[8]={0,1,2,3,4,3,2,1};
    unsigned reverse[8]={0,0,0,0,1,0,0,0};
    uint16_t ticks=0,frames=0;
    assert(tick_divisor!=0 && poll!=NULL);
    for(;;) {
        JxInputEvent event;
        if(poll(input_opaque,&event)!=0 || event.type!=8) continue;
        ticks=(uint16_t)(ticks+1);
        if(ticks%tick_divisor!=0) continue;
        for(unsigned row=0;row<192;row++) {
            uint16_t shift=shifts[row%8];
            JxFarPointer destination=wave_pointer(work,row*320u);
            (void)jx_memory_fill(c,destination,0,shift);
            JxFarPointer from=wave_pointer(source,row*320u);
            destination=wave_pointer(destination,shift);
            (void)jx_memory_copy(c,destination,from,(uint16_t)(320-shift));
        }
        for(unsigned lane=0;lane<8;lane++) {
            if(reverse[lane]) {
                shifts[lane]--;
                if(shifts[lane]==0) reverse[lane]=0;
            } else {
                shifts[lane]++;
                if(shifts[lane]==4) reverse[lane]=1;
            }
        }
        uint16_t page=(uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2);
        (void)JX_CALL(c,0x1832,0x1810,page);
        (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,192,work.offset,work.segment);
        page=(uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2);
        (void)JX_CALL(c,0x1832,0x1836,page);
        jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
        frames=(uint16_t)(frames+1);
        if(frames>frame_limit) {
            (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,source.offset,source.segment);
            return;
        }
    }
}
