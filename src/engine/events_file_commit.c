#include "event_engine.h"
/* 1000:32F4 / shared 05E6. Carry errors set errno, then return only AL. */
uint16_t jx_dos_commit(JxEventContext *c,uint16_t handle,
    JxDosCloseService service,void *opaque) {
    JxDosWriteResult result={0,0};service(opaque,handle,&result);
    if(result.carry==0)return 0;
    (void)jx_dos_error(c,result.ax);
    return result.ax&255;
}
/* 1000:2AF6. Version comparison is signed; success does not clear errno. */
uint16_t jx_file_commit(JxEventContext *c,uint16_t handle) {
    if(jx_signed_word(handle)<0 || jx_signed_word(jx_word(c,0x131c))<=jx_signed_word(handle)) {
        jx_set_word(c,0x130c,9);return 0xffff;
    }
    if(jx_signed_word(jx_word(c,0x1314))<0x31e)return 0;
    if((jx_ds(c)[(uint16_t)(0x131e + handle)]&1)!=0) {
        uint16_t result=(uint16_t)JX_CALL(c,0x1000,0x32f4,handle);
        if(result==0)return 0;
        jx_set_word(c,0x131a,result);
    }
    jx_set_word(c,0x130c,9);
    return 0xffff;
}
