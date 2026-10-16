#include "event_engine.h"
/* Thin resident callbacks; the original overlay/engine address stays explicit. */

/* 1F2A:DD0A. */
void jx_callback_1f2a_dd0a(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0006,npc);
}

/* 1F2A:DDEA. */
void jx_callback_1f2a_ddea(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0018,npc);
}

/* 1F2A:DFC0. */
void jx_callback_1f2a_dfc0(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0024,npc);
}

/* 1F2A:DFD6. */
void jx_callback_1f2a_dfd6(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x002a,npc);
}

/* 1F2A:DFEC. */
void jx_callback_1f2a_dfec(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0030,npc);
}

/* 1F2A:E002. */
void jx_callback_1f2a_e002(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0036,npc);
}

/* 1F2A:E018. */
void jx_callback_1f2a_e018(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x003c,npc);
}

/* 1F2A:E166. */
void jx_callback_1f2a_e166(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0048,npc);
}

/* 1F2A:E17C. */
void jx_callback_1f2a_e17c(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x004e,npc);
}

/* 1F2A:E27C. */
void jx_callback_1f2a_e27c(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0060,npc);
}

/* 1F2A:E292. */
void jx_callback_1f2a_e292(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0066,npc);
}

/* 1F2A:E2CA. */
void jx_callback_1f2a_e2ca(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x006c,npc);
}

/* 1F2A:E404. */
void jx_callback_1f2a_e404(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x008a,npc);
}

/* 1F2A:E43C. */
void jx_callback_1f2a_e43c(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0090,npc);
}

/* 1F2A:E452. */
void jx_callback_1f2a_e452(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0096,npc);
}

/* 1F2A:E49E. */
void jx_callback_1f2a_e49e(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x009c,npc);
}

/* 1F2A:E4CA. */
void jx_callback_1f2a_e4ca(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00a2,npc);
}

/* 1F2A:E4E0. */
void jx_callback_1f2a_e4e0(JxEventContext *c) {
    (void)JX_CALL0(c,0x17d2,0x00a8);
}

/* 1F2A:E588. */
void jx_callback_1f2a_e588(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00b4,npc);
}

/* 1F2A:E59E. */
void jx_callback_1f2a_e59e(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00ba,npc);
}

/* 1F2A:E662. */
void jx_callback_1f2a_e662(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00d2,npc);
}

/* 1F2A:E702. */
void jx_callback_1f2a_e702(JxEventContext *c) {
    (void)JX_CALL0(c,0x17d2,0x00e4);
}

/* 1F2A:E780. */
void jx_callback_1f2a_e780(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00ea,npc);
}

/* 1F2A:E980. */
void jx_callback_1f2a_e980(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00f6,npc);
}

/* 1F2A:E9C2. */
void jx_callback_1f2a_e9c2(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x00fc,npc);
}

/* 1F2A:E9D8. */
void jx_callback_1f2a_e9d8(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0102,npc);
}

/* 1F2A:EA96. */
void jx_callback_1f2a_ea96(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0108,npc);
}

/* 1F2A:EB5C. */
void jx_callback_1f2a_eb5c(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0114,npc);
}

/* 1F2A:ED94. */
void jx_callback_1f2a_ed94(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x011a,npc);
}

/* 1F2A:EEDE. */
void jx_callback_1f2a_eede(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0126,npc);
}

/* 1F2A:EFD4. */
void jx_callback_1f2a_efd4(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x012c,npc);
}

/* 1F2A:EFEA. */
void jx_callback_1f2a_efea(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0132,npc);
}

/* 1F2A:F32E. */
void jx_callback_1f2a_f32e(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x013e,npc);
}

/* 1F2A:FCFC. */
void jx_callback_1f2a_fcfc(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x014a,npc);
}

/* 1F2A:FE0C. */
void jx_callback_1f2a_fe0c(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0150,npc);
}

/* 1F2A:FF8E. */
void jx_callback_1f2a_ff8e(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x015c,npc);
}

/* 2F28:0204. */
void jx_callback_2f28_0204(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0162,npc);
}

/* 2F28:021A. */
void jx_callback_2f28_021a(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0168,npc);
}

/* 2F28:0EF8. */
void jx_callback_2f28_0ef8(JxEventContext *c) {
    (void)JX_CALL0(c,0x17d2,0x018c);
}

/* 2F28:0F5A. */
void jx_callback_2f28_0f5a(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x0192,npc);
}

/* 2F28:16E6. */
void jx_callback_2f28_16e6(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x019e,npc);
}

/* 2F28:18D6. */
void jx_callback_2f28_18d6(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x01a4,npc);
}

/* 2F28:1B32. */
void jx_callback_2f28_1b32(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x01b0,npc);
}

/* 2F28:21C8. */
void jx_callback_2f28_21c8(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x01bc,npc);
}

/* 2F28:21F4. */
void jx_callback_2f28_21f4(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x01c2,npc);
}

/* 2F28:2544. */
void jx_callback_2f28_2544(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x17d2,0x01c8,npc);
}

/* 2F28:34F4. */
void jx_callback_2f28_34f4(JxEventContext *c,uint16_t npc) {
    (void)JX_CALL(c,0x1512,0x016a,npc);
}
