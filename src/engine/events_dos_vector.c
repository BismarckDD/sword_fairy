#include "event_engine.h"
/* 1000:3302, DOS35h returns ES:BX as the original DX:AX far address. */
uint32_t jx_dos_get_vector(JxDosGetVectorService service,void *opaque,
    uint16_t vector) {
    return service(opaque,(uint8_t)vector);
}
/* 1000:3314. AH=25h discards the vector argument's high byte. The original
 * always returns AX=0 after the DOS service, independently of carry. */
uint16_t jx_dos_set_vector(JxDosVectorService service,void *opaque,
    uint16_t vector,JxFarPointer handler) {
    service(opaque,(uint8_t)vector,handler);
    return 0;
}
