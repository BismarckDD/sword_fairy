#include "event_engine.h"
/* 1000:2B50 / 2B5E. Safe host device callbacks model IN/OUT; there is no
 * direct hardware access. Both return AL zero-extended, and output uses
 * only the low byte of the original word argument. */
uint16_t jx_port_read_byte(const JxPortDevice *device,uint16_t port) {
    return device->read(device->opaque,port);
}
uint16_t jx_port_write_byte(const JxPortDevice *device,uint16_t port,uint16_t value) {
    uint8_t byte=(uint8_t)value;
    device->write(device->opaque,port,byte);
    return byte;
}
