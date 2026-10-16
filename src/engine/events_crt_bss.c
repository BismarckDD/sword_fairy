#include "event_engine.h"
/* Original CRT 1000:009C–00A7: ES=SS, CLD, clear DS2724..D48F.
 * The unpacking snapshot precedes this step and contains compressed-loader
 * residue in the uninitialized data area. Preserve initialized DS below2724. */
void jx_crt_clear_bss(JxEventContext *c) {
    for(uint16_t offset=0x2724;offset<0xd490;offset++)jx_ds(c)[offset]=0;
}
