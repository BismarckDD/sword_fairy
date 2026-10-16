#include "event_engine.h"

static void irq_word(JxEventContext *c,JxFarPointer event,uint16_t offset,
    uint16_t value) {
    uint32_t address=((uint32_t)event.segment*16+(uint16_t)(event.offset+offset))&0xfffff;
    c->memory[address]=(uint8_t)value;c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 1F2A:7038..71DC, observed IRQ9 body. Hardware return flags/registers and
 * producer serialization belong to the host IRQ wrapper. Fixed original DS
 * is independent of interrupted DS. Event words2/3 are untouched locals:
 * preserve caller scratch bytes instead of inventing values for them. */
void jx_keyboard_irq(JxEventContext *context,const JxPortDevice *ports,
    void (*interrupt_disable)(void *),void *irq_opaque,JxFarPointer event) {
    JxEventContext fixed=*context;fixed.data_segment=0x39e3;
    JxEventContext *c=&fixed;
    interrupt_disable(irq_opaque);
    uint8_t scan=ports->read(ports->opaque,0x60);
    uint8_t acknowledge=ports->read(ports->opaque,0x61);
    ports->write(ports->opaque,0x61,(uint8_t)(acknowledge|0x80));
    ports->write(ports->opaque,0x61,(uint8_t)(acknowledge&0x7f));
    uint16_t translated=(uint8_t)JX_CALL(c,0x1f2a,0x6eb8,(uint16_t)(scan&0x7f));
    irq_word(c,event,2,translated);
    if(scan!=0xe0 && translated!=0) {
        uint16_t released=(scan&0x80)?1:0;
        irq_word(c,event,0,released);
        if(translated==0x38)jx_set_word(c,0x04ee,released?0:1);
        else if(translated==0x1d)jx_set_word(c,0x04f0,released?0:1);
        else {
            uint16_t direction=0xffff;
            if(translated==0x48)direction=0;
            else if(translated==0x50)direction=1;
            else if(translated==0x4b)direction=2;
            else if(translated==0x4d)direction=3;
            if(direction!=0xffff)(void)JX_CALL(c,0x1f2a,0x6f20,direction,released?0:1);
        }
        (void)JX_CALL(c,0x1f2a,0xc15a,event.offset,event.segment);
    }
    ports->write(ports->opaque,0x20,0x20);
}
/* 2F28:A4BC..A561, IRQ1C body. Counts expire on equality to zero, not
 * signed <=0. Enqueue callback may mutate reload/slot flags; reread them. */
void jx_timer_irq(JxEventContext *context,JxFarPointer event) {
    JxEventContext fixed=*context;fixed.data_segment=0x39e3;
    JxEventContext *c=&fixed;
    for(unsigned index=0;index<5;index++) {
        uint16_t slot=(uint16_t)(0x2b5c+6*index);
        if(jx_ds(c)[slot+4]==0 || jx_ds(c)[slot+5]==0)continue;
        uint16_t remaining=(uint16_t)(jx_word(c,slot)-1);
        jx_set_word(c,slot,remaining);
        if(remaining==0) {
            irq_word(c,event,0,8);irq_word(c,event,2,(uint16_t)index);
            (void)JX_CALL(c,0x1f2a,0xc15a,event.offset,event.segment);
            jx_set_word(c,slot,jx_word(c,(uint16_t)(slot+2)));
        }
    }
}
