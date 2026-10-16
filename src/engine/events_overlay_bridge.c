#include <assert.h>
#include "event_engine.h"
/* Original overlay jump records; unknown targets fall through. */
static uint32_t overlay_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxEngineBridge *b=(JxEngineBridge *)opaque;
    if(segment==0x17d2) switch(offset) {
    case 0x0006:assert(n<=1);jx_event_005_0594(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x000c:assert(n<=1);jx_event_005_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0012:assert(n<=1);jx_event_006_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0018:assert(n<=1);jx_event_006_0132(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x001e:assert(n<=1);jx_event_010_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0024:assert(n<=1);jx_event_010_0066(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x002a:assert(n<=1);jx_event_010_00c0(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0030:assert(n<=1);jx_event_010_00ee(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0036:assert(n<=1);jx_event_010_01ac(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x003c:assert(n<=1);jx_event_010_0226(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0042:assert(n<=1);jx_event_010_028c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0048:assert(n<=1);jx_event_011_001e(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x004e:assert(n<=1);jx_event_011_004c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0054:assert(n<=1);jx_event_011_00c8(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x005a:assert(n<=1);jx_event_011_014e(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0060:assert(n<=1);jx_event_014_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0066:assert(n<=1);jx_event_014_0064(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x006c:assert(n<=1);jx_event_015_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0072:assert(n<=1);jx_event_017_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0078:assert(n<=1);jx_event_017_008c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x007e:assert(n<=1);jx_event_017_00f4(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0084:assert(n<=1);jx_event_018_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x008a:assert(n<=1);jx_event_018_00aa(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0090:assert(n<=1);jx_event_019_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0096:assert(n<=1);jx_event_019_0376(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x009c:assert(n<=1);jx_event_022_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00a2:assert(n<=1);jx_event_023_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00a8:assert(n<=1);jx_event_023_00ba(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00ae:assert(n<=1);jx_event_024_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00b4:assert(n<=1);jx_event_025_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00ba:assert(n<=1);jx_event_025_005c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00c0:assert(n<=1);jx_event_025_0126(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00c6:assert(n<=1);jx_event_025_0162(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00cc:assert(n<=1);jx_event_026_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00d2:assert(n<=1);jx_event_026_009c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00d8:assert(n<=1);jx_event_027_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00de:assert(n<=1);jx_event_027_0072(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00e4:assert(n<=1);jx_event_027_0124(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00ea:assert(n<=1);jx_event_027_01e4(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00f0:assert(n<=1);jx_event_027_02ca(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00f6:assert(n<=1);jx_event_011_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x00fc:assert(n<=1);jx_event_031_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0102:assert(n<=1);jx_event_031_002c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0108:assert(n<=1);jx_event_033_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x010e:assert(n<=1);jx_event_035_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0114:assert(n<=1);jx_event_035_00e8(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x011a:assert(n<=1);jx_event_037_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0120:assert(n<=1);jx_event_037_0064(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0126:assert(n<=1);jx_event_043_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x012c:assert(n<=1);jx_event_046_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0132:assert(n<=1);jx_event_046_00be(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0138:assert(n<=1);jx_event_057_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x013e:assert(n<=1);jx_event_058_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0144:assert(n<=1);jx_event_072_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x014a:assert(n<=1);jx_event_083_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0150:assert(n<=1);jx_event_086_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0156:assert(n<=1);jx_event_086_0130(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x015c:assert(n<=1);jx_event_090_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0162:assert(n<=1);jx_event_101_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0168:assert(n<=1);jx_event_101_009e(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x016e:assert(n<=1);jx_event_103_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0174:assert(n<=1);jx_event_103_00fa(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x017a:assert(n<=1);jx_event_103_015a(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0180:assert(n<=1);jx_event_111_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0186:assert(n<=1);jx_event_123_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x018c:assert(n<=1);jx_event_130_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0192:assert(n<=1);jx_event_131_011c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x0198:assert(n<=1);jx_event_131_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x019e:assert(n<=1);jx_event_152_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01a4:assert(n<=1);jx_event_156_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01aa:assert(n<=1);jx_event_159_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01b0:assert(n<=1);jx_event_163_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01b6:assert(n<=1);jx_event_178_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01c2:assert(n<=1);jx_event_181_0000(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01c8:assert(n<=1);jx_event_194_088c(b->context,n?(int16_t)a[0]:0);return 0;
    case 0x01ce:assert(n<=1);jx_event_197_0000(b->context,n?(int16_t)a[0]:0);return 0;
    default:break;
    }
    return b->fallback(b->fallback_opaque,segment,offset,a,n);
}
void jx_overlay_event_bridge_install(JxEventContext *c,JxEngineBridge *b) {
    assert(c && b && c->call && c->call!=overlay_dispatch);
    b->context=c;b->fallback=c->call;b->fallback_opaque=c->opaque;
    c->call=overlay_dispatch;c->opaque=b;
}
void jx_overlay_event_bridge_remove(JxEventContext *c,JxEngineBridge *b) {
    assert(c->call==overlay_dispatch && c->opaque==b);
    c->call=b->fallback;c->opaque=b->fallback_opaque;
}
