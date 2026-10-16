#include "event_engine.h"
#include <assert.h>
static JxFarPointer calendar_arg(const uint16_t *a) {JxFarPointer p={a[0],a[1]};return p;}
static JxFarPointer calendar_work(JxCalendarBridge *b,uint16_t field) {
    JxFarPointer p={(uint16_t)(b->private_work.offset+field),b->private_work.segment};return p;
}
static uint32_t calendar_dispatch(void *opaque,uint16_t segment,uint16_t off,const uint16_t *a,size_t n) {
    JxCalendarBridge *b=opaque;JxEventContext *c=b->base.context;
    if(segment==0x1f2a && off==0x720e) {assert(n==1);jx_wait_milliseconds(c,a[0]);return 0;}
    if(segment==0x1000) {
        switch(off) {
        case 0x24ae:assert(n==2);return jx_string_length(c,calendar_arg(a));
        case 0x24c8:assert(n==5);return jx_string_copy_bounded(c,calendar_arg(a),calendar_arg(a+2),a[4]);
        case 0x24f2:assert(n==5);return (uint16_t)jx_string_compare_bounded(c,calendar_arg(a),calendar_arg(a+2),a[4]);
        case 0x252e:assert(n==2);return jx_atol32(c,calendar_arg(a));
        case 0x26a4:assert(n==2);return jx_environment_get(c,calendar_arg(a));
        case 0x2b6c:assert(n==2);return jx_ftime(c,calendar_arg(a),calendar_work(b,0));
        case 0x2c7e:assert(n==0);jx_timezone_ensure_initialized(c);return 0;
        case 0x2c90:assert(n==0);jx_timezone_initialize(c);return 0;
        case 0x2dda:assert(n==2);return jx_calendar_is_daylight(c,calendar_arg(a));
        case 0x2ea8:assert(n==6);return jx_calendar_to_seconds(c,a[0],a[1],a[2],a[3],a[4],a[5],calendar_work(b,40));
        case 0x2fa0:assert(n==0);return jx_clock_milliseconds(c,calendar_work(b,64));
        case 0x332a:assert(n==2);return jx_dos_get_date(c,calendar_arg(a),b->service,b->service_opaque);
        case 0x3346:assert(n==2);return jx_dos_get_time(c,calendar_arg(a),b->service,b->service_opaque);
        case 0x3384:case 0x3450: {
            assert(n==4);uint32_t result;
            uint32_t left=a[0]|((uint32_t)a[1]<<16),right=a[2]|((uint32_t)a[3]<<16);
            int ok=off==0x3384?jx_divide_signed32(left,right,&result):jx_remainder_signed32(left,right,&result);
            if(ok)return result;
            break;
        }
        default:break;
        }
    }
    return b->base.fallback(b->base.fallback_opaque,segment,off,a,n);
}
void jx_calendar_bridge_install(JxEventContext *c,JxCalendarBridge *b,JxDosCalendarService service,void *opaque,JxFarPointer work) {
    assert(c && b && c->call && service && c->call!=calendar_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->service=service;b->service_opaque=opaque;b->private_work=work;c->call=calendar_dispatch;c->opaque=b;
}
void jx_calendar_bridge_remove(JxEventContext *c,JxCalendarBridge *b) {
    assert(c && b && c->call==calendar_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
