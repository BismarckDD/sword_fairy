#include "event_engine.h"
/* 1832:2816. Forward REP MOVSB with fixed DS/ES segments. Each offset
 * wraps independently at FFFF; there is no huge-pointer segment carry.
 * Read then write each byte to preserve forward-overlap propagation. */
void jx_memory_copy_fixed(JxEventContext *c,JxFarPointer destination,JxFarPointer source,uint16_t count) {
    for(unsigned i=0;i<count;i++) {
        uint8_t value=jx_far_byte(c,source,0);
        jx_far_set_byte(c,destination,0,value);
        source.offset++;destination.offset++;
    }
}
