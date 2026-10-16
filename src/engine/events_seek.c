#include "event_engine.h"
/* 1000:0606, near shared error mapper. AL records raw DOS error even when
 * AH supplies a synthetic CRT errno. Table bytes are sign-extended. */
uint16_t jx_dos_error(JxEventContext *c,uint16_t error) {
    uint8_t code=(uint8_t)error,crt=(uint8_t)(error>>8);
    jx_ds(c)[0x131a]=code;
    if(!crt) {
        if(jx_ds(c)[0x1315]>=3 && code>=32 && code<34) code=5;
        else if(code>19) code=19;
        crt=jx_ds(c)[0x1358+code];
    }
    uint16_t result=(uint16_t)jx_signed_byte(crt);
    jx_set_word(c,0x130c,result);return result;
}
static uint32_t seek_error(JxEventContext *c,uint16_t error) {
    (void)jx_dos_error(c,error);return 0xffffffffu;
}
static uint32_t seek_position(JxDosSeekResult result) {
    return result.ax|((uint32_t)result.dx<<16);
}
static JxDosSeekResult seek_call(JxDosSeekService service,void *opaque,uint16_t handle,uint8_t origin,uint32_t offset) {
    JxDosSeekResult result={0,0,0};service(opaque,handle,origin,offset,&result);return result;
}
/* 1000:1FA8. Negative seek validation can move the DOS file pointer before
 * the final request. Preserve ignored carry on the intermediate EOF query. */
uint32_t jx_file_seek(JxEventContext *c,uint16_t handle,uint32_t offset,
    uint16_t origin,JxDosSeekService service,void *opaque) {
    if(handle>=jx_word(c,0x131c)) return seek_error(c,0x0900);
    if(jx_word(c,0x15d0) && (offset&0x80000000u)) {
        if(!origin) return seek_error(c,0x1600);
        JxDosSeekResult current=seek_call(service,opaque,handle,1,0);
        if(current.carry) return seek_error(c,current.ax);
        if(origin&2) {
            JxDosSeekResult end=seek_call(service,opaque,handle,2,0);
            if((seek_position(end)+offset)&0x80000000u) {
                (void)seek_call(service,opaque,handle,0,seek_position(current));
                return seek_error(c,0x1600);
            }
        } else if((seek_position(current)+offset)&0x80000000u) {
            return seek_error(c,0x1600);
        }
    }
    JxDosSeekResult result=seek_call(service,opaque,handle,(uint8_t)origin,offset);
    if(result.carry) return seek_error(c,result.ax);
    jx_ds(c)[(uint16_t)(0x131e + handle)]&=(uint8_t)~2;
    return seek_position(result);
}
