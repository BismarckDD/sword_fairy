#include "event_engine.h"

/* 1F2A:C15A, serialized producer, original MOVSW precondition DF=0.
 * Disabled input returns success without touching the source. The next tail
 * uses signed word remainder; copy then re-read tail to retain alias effects. */
int jx_input_enqueue(JxEventContext *c,JxFarPointer source) {
    if(jx_word(c,0x0868)==0)return 0;
    uint16_t tail=jx_word(c,0x0866);
    uint16_t next=(uint16_t)(jx_signed_word((uint16_t)(tail+1))%64);
    if(next==jx_word(c,0x0864))return -1;
    uint16_t destination=(uint16_t)(0x2956+tail*8u);
    for(unsigned i=0;i<4;i++) {
        uint16_t value=jx_far_word(c,source.segment,(uint16_t)(source.offset+2*i));
        jx_set_word(c,(uint16_t)(destination+2*i),value);
    }
    tail=jx_word(c,0x0866);
    jx_set_word(c,0x0866,(uint16_t)(jx_signed_word((uint16_t)(tail+1))%64));
    return 0;
}

/* 1F2A:C1F2. A 64-entry ring of eight-byte input/timer events.
 * The original masks interrupts during dequeue. Hosts must serialize queue
 * producers and this operation; a portable C function cannot mask DOS IRQs. */
int jx_input_dequeue(JxEventContext *c,uint16_t destination_offset,uint16_t destination_segment) {
    uint16_t head=jx_word(c,0x0864);
    if(jx_word(c,0x0866)==head) return -1;
    uint16_t source=(uint16_t)(0x2956 + head*8u);
    for(unsigned i=0;i<4;i++) {
        uint16_t value=jx_word(c,(uint16_t)(source+2*i));
        uint16_t offset=(uint16_t)(destination_offset+2*i);
        uint32_t address=((uint32_t)destination_segment*16+offset)&0xfffff;
        c->memory[address]=(uint8_t)value;
        c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
    }
    /* Re-read after copy: a caller can alias the queue indices. */
    uint16_t next=(uint16_t)(jx_word(c,0x0864)+1);
    jx_set_word(c,0x0864,(uint16_t)(jx_signed_word(next)%64));
    return 0;
}

/* 1F2A:C24E. The original resets both indices, without clearing payloads. */
void jx_input_clear(JxEventContext *c) {
    jx_set_word(c,0x0866,0);
    jx_set_word(c,0x0864,0);
}

/* Native-local variant of C1F2; event must not alias the emulated memory.
 * This adapter does not invent timer events or schedule an IRQ producer. */
int jx_input_poll_queue(void *context,JxInputEvent *event) {
    JxEventContext *c=context;
    uint16_t head=jx_word(c,0x0864);
    if(head==jx_word(c,0x0866)) return -1;
    uint16_t source=(uint16_t)(0x2956+head*8u);
    event->type=jx_word(c,source);
    event->value1=jx_word(c,(uint16_t)(source+2));
    event->value2=jx_word(c,(uint16_t)(source+4));
    event->value3=jx_word(c,(uint16_t)(source+6));
    jx_set_word(c,0x0864,(uint16_t)(jx_signed_word((uint16_t)(head+1))%64));
    return 0;
}

/* 1F2A:2DBC. Even a zero duration waits for one timer event. The signed
 * exit comparison follows a wrapping word decrement: 0x8000 waits 32768
 * ticks, since its first decrement becomes positive 0x7fff. */
void jx_delay(JxEventContext *c,uint16_t duration,JxInputPoll poll,void *opaque) {
    jx_input_clear(c);
    for(;;) {
        JxInputEvent event;
        if(poll(opaque,&event)!=0 || event.type!=8) continue;
        jx_music_poll(c);
        duration=(uint16_t)(duration-1);
        if(jx_signed_word(duration)<=0) return;
    }
}

