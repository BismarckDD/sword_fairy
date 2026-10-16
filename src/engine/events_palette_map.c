#include "event_engine.h"
#include <assert.h>
#include <float.h>

/* 2F28:A72E. Exact zero wins immediately even when a previous signed
 * overflow produced a negative distance. This differs from A7FE's early
 * threshold check. An indeterminate original uninitialized result is -1. */
int jx_palette_nearest_exact(JxEventContext *c,uint8_t red,uint8_t green,uint8_t blue,JxFarPointer palette) {
    int best_distance=20000,best=-1;
    for(int index=0;index<256;index++) {
        int dr=(int)jx_far_byte(c,palette,0)-red;
        int dg=(int)jx_far_byte(c,palette,1)-green;
        int db=(int)jx_far_byte(c,palette,2)-blue;
        int distance=jx_signed_word((uint16_t)(dr*dr+dg*dg+db*db));
        if(distance==0) return index;
        if(distance<best_distance) {best_distance=distance;best=index;}
        palette.offset=(uint16_t)(palette.offset+3);
    }
    return best;
}

/* Keep the original x87 extended-precision operation boundaries. Zig's
 * Windows GNU C target provides this long double representation. A compiler
 * with 53-bit long double must not silently change boundary rounding. */
_Static_assert(LDBL_MANT_DIG==64 && LDBL_MAX_EXP==16384,"80-bit extended long double required");
static uint8_t mapped_channel(uint8_t source,uint8_t target,float weight) {
    volatile long double complement=1.0L-(long double)weight;
    volatile long double left=(long double)source*complement;
    volatile long double right=(long double)target*(long double)weight;
    volatile long double sum=left+right;
    /* 47E8 temporarily selects truncation and FISTP qword, then restores
     * the control word. Integer-indefinite (invalid/overflow) has low byte 0. */
    if(!(sum>=-9223372036854775808.0L && sum<9223372036854775808.0L)) return 0;
    return (uint8_t)(int64_t)sum;
}

/* 2F28:A8D0. RGB target is passed as a four-byte value; its fourth byte is
 * unused. Source and output offsets wrap in their fixed segments. Palette
 * aliasing is intentional: nearest matching rereads the live original base. */
int jx_palette_build_map(JxEventContext *c,JxFarPointer palette,uint32_t target,
    JxFarPointer output,float weight) {
    if(!(weight>0.0f)) return -1;
    JxFarPointer source=palette;
    for(unsigned index=0;index<256;index++) {
        uint8_t red=mapped_channel(jx_far_byte(c,source,0),(uint8_t)target,weight);
        uint8_t green=mapped_channel(jx_far_byte(c,source,1),(uint8_t)(target>>8),weight);
        uint8_t blue=mapped_channel(jx_far_byte(c,source,2),(uint8_t)(target>>16),weight);
        int nearest=jx_palette_nearest_exact(c,red,green,blue,palette);
        assert(nearest>=0); /* original result is indeterminate otherwise */
        jx_far_set_byte(c,output,(uint16_t)index,(uint8_t)nearest);
        source.offset=(uint16_t)(source.offset+3);
    }
    return 0;
}
