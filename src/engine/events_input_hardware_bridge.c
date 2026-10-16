#include "event_engine.h"
#include <assert.h>

/* IRQ address routes are logical host interrupt requests, not far calls
 * emulating IRET. Host retains CPU flags/registers and serializes producers. */
static uint32_t input_hardware_dispatch(void *opaque,uint16_t segment,
    uint16_t offset,const uint16_t *a,size_t n) {
    JxInputHardwareBridge *b=opaque;JxEventContext fixed=*b->base.context;
    if(b->active)fixed.data_segment=0x39e3;
    JxEventContext *c=&fixed;
    if(segment==0x1000) {
        if(offset==0x3302){assert(n==1);return jx_dos_get_vector(b->device.get_vector,b->device.opaque,a[0]);}
        if(offset==0x3314){assert(n==3);return jx_dos_set_vector(b->device.set_vector,b->device.opaque,a[0],(JxFarPointer){a[1],a[2]});}
    }
    if(segment==0x1f2a) {
        switch(offset) {
        case 0x5f60:assert(n==0);jx_keyboard_initialize(c,b->device.keyboard_bios,b->device.opaque);return 0;
        case 0x5f78:assert(n==0);jx_timer_initialize(c);return 0;
        case 0x6d46:assert(n==0);jx_keyboard_install_vector(c);return 0;
        case 0x6dac:assert(n==1);return jx_keyboard_randomize_direction(c,a[0]);
        case 0x6e62:assert(n==0);return jx_keyboard_read_direction(c);
        case 0x6e8e:assert(n==0);jx_keyboard_restore_vector(c);return 0;
        case 0x6eb8:assert(n==1);return jx_keyboard_translate(c,a[0]);
        case 0x6f20:assert(n==2);return jx_keyboard_set_direction(c,a[0],a[1]);
        case 0xc15a:assert(n==2);return (uint16_t)jx_input_enqueue(c,(JxFarPointer){a[0],a[1]});
        case 0xc1f2:assert(n==2);return (uint16_t)jx_input_dequeue(c,a[0],a[1]);
        case 0xc24e:assert(n==0);jx_input_clear(c);return 0;
        case 0x7038:
            assert(n==0 && !b->active);b->active=1;
            jx_keyboard_irq(c,&b->ports,b->device.interrupt_disable,b->device.opaque,b->irq_event);
            b->active=0;return 0;
        default:break;
        }
    }
    if(segment==0x2f28) {
        switch(offset) {
        case 0xa36e:assert(n==0);jx_timer_install_vector(c);return 0;
        case 0xa3aa:assert(n==0);jx_timer_restore_vector(c);return 0;
        case 0xa3d0:assert(n==1);return jx_timer_add_callback(c,a[0]);
        case 0xa468:assert(n==1);jx_timer_remove_callback(c,a[0]);return 0;
        case 0xa4bc:
            assert(n==0 && !b->active);b->active=1;
            jx_timer_irq(c,b->irq_event);b->active=0;return 0;
        default:break;
        }
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_input_hardware_bridge_install(JxEventContext *c,JxInputHardwareBridge *b,
    const JxPortDevice *ports,const JxInputHardwareDevice *device,JxFarPointer event) {
    assert(c && b && ports && device && c->call && c->call!=input_hardware_dispatch);
    assert(ports->read && ports->write && device->keyboard_bios && device->get_vector && device->set_vector && device->interrupt_disable);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->ports=*ports;b->device=*device;b->irq_event=event;b->active=0;
    c->call=input_hardware_dispatch;c->opaque=b;
}
void jx_input_hardware_bridge_remove(JxEventContext *c,JxInputHardwareBridge *b) {
    assert(c->call==input_hardware_dispatch && c->opaque==b && !b->active);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
