#include "event_engine.h"
/* Shared 1000:05DE return path. Errors record the raw DOS code. */
static uint16_t lifecycle_result(JxEventContext *c,JxDosWriteResult result) {
    if(!result.carry) return 0;
    (void)jx_dos_error(c,result.ax);return 65535;
}
/* 1000:1F88. Clear the live handle table only after successful DOS close. */
uint16_t jx_file_close(JxEventContext *c,uint16_t handle,JxDosCloseService service,void *opaque) {
    if(handle>=jx_word(c,0x131c)) return lifecycle_result(c,(JxDosWriteResult){0x0900,1});
    JxDosWriteResult result={0,0};service(opaque,handle,&result);
    if(!result.carry) jx_ds(c)[(uint16_t)(0x131e + handle)]=0;
    return lifecycle_result(c,result);
}
/* 1000:4C80. The supplied service owns pathname interpretation. */
uint16_t jx_file_delete(JxEventContext *c,JxFarPointer name,JxDosDeleteService service,void *opaque) {
    JxDosWriteResult result={0,0};service(opaque,name,&result);
    return lifecycle_result(c,result);
}
