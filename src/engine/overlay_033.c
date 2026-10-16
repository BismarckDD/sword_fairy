/* MAIN.EXE overlay #33, header at file 36c00, code at file 36e00.
 * C reconstruction of all 143 bytes of its one event handler.
 * Unknown engine helper retains its address; callbacks must implement engine/UI.
 * This is an event-function reconstruction, not a runnable game.
 */
#include <stdint.h>

typedef struct {
    void (*call_2f28_648e)(int16_t npc, uint16_t offset, uint16_t segment, int16_t mode);
    void (*grant_item_with_ui)(int16_t item_id, int16_t count);
} JxEventApi;

static uint16_t read16(const uint8_t *p) {
    return (uint16_t)((unsigned)p[0] | ((unsigned)p[1]<<8));
}

void jx_overlay_033(uint8_t ds[65536], int16_t npc_index, const JxEventApi *api) {
    uint16_t state_address=(uint16_t)(0xb9ce + npc_index);
    int state=ds[state_address];
    if (state>=128) state-=256; /* original CBW: signed byte */
    ds[0x346b]=(uint8_t)(state<3 ? 0 : 1);
    ds[0xb830]=5;
    if (state==5) {
        api->call_2f28_648e(npc_index,read16(ds+0x6022),read16(ds+0x6024),1);
        api->call_2f28_648e(npc_index,read16(ds+0x6026),read16(ds+0x6028),1);
        api->grant_item_with_ui(0x34,1);
        api->grant_item_with_ui(0x30,1);
        ds[state_address]=6;
    }
}
