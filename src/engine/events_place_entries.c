#include "event_engine.h"
/* DS:0A6C place-entry table. Exact constant registrations; no VM. */

/* Place 2, 1F2A:DEF6. */
void jx_place_enter_002(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 3, 1F2A:DF76. */
void jx_place_enter_003(JxEventContext *c) {
    jx_set_word(c,0x873e,0xdf00);
    jx_set_word(c,0x8740,0x1f2a);
}

/* Place 4, 1F2A:DF8C. */
void jx_place_enter_004(JxEventContext *c) {
    jx_set_word(c,0x873e,0xdf00);
    jx_set_word(c,0x8740,0x1f2a);
}

/* Place 6, 1F2A:E1E6. */
void jx_place_enter_006(JxEventContext *c) {
    jx_set_word(c,0x893c,0xe166); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
    jx_set_word(c,0x8b48,0xe17c); /* NPC 7, field +52. */
    jx_set_word(c,0x8b4a,0x1f2a); /* NPC 7, field +54. */
    jx_set_word(c,0x8b9e,0xe17c); /* NPC 8, field +52. */
    jx_set_word(c,0x8ba0,0x1f2a); /* NPC 8, field +54. */
    jx_set_word(c,0x8bf4,0xe17c); /* NPC 9, field +52. */
    jx_set_word(c,0x8bf6,0x1f2a); /* NPC 9, field +54. */
    jx_set_word(c,0x8c4a,0xe17c); /* NPC 10, field +52. */
    jx_set_word(c,0x8c4c,0x1f2a); /* NPC 10, field +54. */
    jx_set_word(c,0x8ca0,0xe17c); /* NPC 11, field +52. */
    jx_set_word(c,0x8ca2,0x1f2a); /* NPC 11, field +54. */
    jx_set_word(c,0x6584,0x0039); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x004e); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xe192); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x001c); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x002f); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xe1bc); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
}

/* Place 7, 1F2A:E268. */
void jx_place_enter_007(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 8, 1F2A:E272. */
void jx_place_enter_008(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 9, 1F2A:E2A8. */
void jx_place_enter_009(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xe27c); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0xe292); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
}

/* Place 10, 1F2A:E2E0. */
void jx_place_enter_010(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xe2ca); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 11, 1F2A:E2F6. */
void jx_place_enter_011(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 12, 1F2A:E36E. */
void jx_place_enter_012(JxEventContext *c) {
    jx_set_word(c,0x893c,0xe300); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
    jx_set_word(c,0x8992,0xe32c); /* NPC 2, field +4A. */
    jx_set_word(c,0x8994,0x1f2a); /* NPC 2, field +4C. */
    jx_set_word(c,0x6584,0x000f); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x000b); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xe350); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x000f); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x000c); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xe350); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x000f); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x000d); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xe350); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
}

/* Place 13, 1F2A:E41A. */
void jx_place_enter_013(JxEventContext *c) {
    jx_set_word(c,0x8992,0xe3d8); /* NPC 2, field +4A. */
    jx_set_word(c,0x8994,0x1f2a); /* NPC 2, field +4C. */
    jx_set_word(c,0x89e8,0xe404); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x1f2a); /* NPC 3, field +4C. */
}

/* Place 14, 1F2A:E468. */
void jx_place_enter_014(JxEventContext *c) {
    jx_set_word(c,0x899a,0xe43c); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x1f2a); /* NPC 2, field +54. */
    jx_set_word(c,0x8aea,0xe452); /* NPC 6, field +4A. */
    jx_set_word(c,0x8aec,0x1f2a); /* NPC 6, field +4C. */
}

/* Place 15, 1F2A:E48A. */
void jx_place_enter_015(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 16, 1F2A:E494. */
void jx_place_enter_016(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 17, 1F2A:E4B4. */
void jx_place_enter_017(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xe49e); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 18, 1F2A:E4F0. */
void jx_place_enter_018(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xe4ca); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0xe4ca); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x1f2a); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0xe4ca); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x1f2a); /* NPC 2, field +54. */
    jx_set_word(c,0x89f0,0xe4ca); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x1f2a); /* NPC 3, field +54. */
    jx_set_word(c,0x8a46,0xe4ca); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x1f2a); /* NPC 4, field +54. */
    jx_set_word(c,0x6584,0x0045); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0012); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xe4e0); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
}

/* Place 19, 1F2A:E572. */
void jx_place_enter_019(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xe54e); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 21, 1F2A:E678. */
void jx_place_enter_021(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xe648); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0xe662); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x1f2a); /* NPC 1, field +54. */
}

/* Place 23, 1F2A:E944. */
void jx_place_enter_023(JxEventContext *c) {
    jx_set_word(c,0x6584,0x007e); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x005c); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xe928); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
}

/* Place 24, 1F2A:E996. */
void jx_place_enter_024(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xe966); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x8992,0xe980); /* NPC 2, field +4A. */
    jx_set_word(c,0x8994,0x1f2a); /* NPC 2, field +4C. */
}

/* Place 25, 1F2A:E9B8. */
void jx_place_enter_025(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 26, 1F2A:E9EE. */
void jx_place_enter_026(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xe9d8); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x89e8,0xe9c2); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x1f2a); /* NPC 3, field +4C. */
}

/* Place 27, 1F2A:EA10. */
void jx_place_enter_027(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 28, 1F2A:EAFA. */
void jx_place_enter_028(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xea1a); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
    jx_set_word(c,0x88e6,0xea96); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x6584,0x0010); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x000a); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xeaac); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
}

/* Place 29, 1F2A:EB34. */
void jx_place_enter_029(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 30, 1F2A:EB72. */
void jx_place_enter_030(JxEventContext *c) {
    jx_set_word(c,0x6584,0x001a); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x002b); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xeb3e); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x001b); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x002b); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xeb3e); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x001c); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x002b); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xeb3e); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_set_word(c,0x893c,0xeb5c); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
}

/* Place 31, 1F2A:EC72. */
void jx_place_enter_031(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xebee); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0xebee); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
    jx_set_word(c,0x8e46,0xec20); /* NPC 16, field +4A. */
    jx_set_word(c,0x8e48,0x1f2a); /* NPC 16, field +4C. */
    jx_set_word(c,0x8e9c,0xec20); /* NPC 17, field +4A. */
    jx_set_word(c,0x8e9e,0x1f2a); /* NPC 17, field +4C. */
    jx_set_word(c,0x6584,0x0073); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x005b); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xebd0); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0074); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x005b); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xebd0); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0075); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x005b); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xebd0); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x0020); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x0048); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0xec30); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x1f2a); /* Location trigger 3, callback segment */
    jx_set_word(c,0x65a4,0x0021); /* Location trigger 4, x */
    jx_set_word(c,0x65a6,0x0048); /* Location trigger 4, y */
    jx_set_word(c,0x65a8,0xec30); /* Location trigger 4, callback offset */
    jx_set_word(c,0x65aa,0x1f2a); /* Location trigger 4, callback segment */
    jx_set_word(c,0x65ac,0x0022); /* Location trigger 5, x */
    jx_set_word(c,0x65ae,0x0048); /* Location trigger 5, y */
    jx_set_word(c,0x65b0,0xec30); /* Location trigger 5, callback offset */
    jx_set_word(c,0x65b2,0x1f2a); /* Location trigger 5, callback segment */
    jx_set_word(c,0x65b4,0x0023); /* Location trigger 6, x */
    jx_set_word(c,0x65b6,0x0048); /* Location trigger 6, y */
    jx_set_word(c,0x65b8,0xec30); /* Location trigger 6, callback offset */
    jx_set_word(c,0x65ba,0x1f2a); /* Location trigger 6, callback segment */
    jx_set_word(c,0x65bc,0x0024); /* Location trigger 7, x */
    jx_set_word(c,0x65be,0x0048); /* Location trigger 7, y */
    jx_set_word(c,0x65c0,0xec30); /* Location trigger 7, callback offset */
    jx_set_word(c,0x65c2,0x1f2a); /* Location trigger 7, callback segment */
}

/* Place 33, 1F2A:EE34. */
void jx_place_enter_033(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 34, 1F2A:EE3E. */
void jx_place_enter_034(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 35, 1F2A:EE9C. */
void jx_place_enter_035(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xee48); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x8992,0xee7e); /* NPC 2, field +4A. */
    jx_set_word(c,0x8994,0x1f2a); /* NPC 2, field +4C. */
    jx_set_word(c,0x89e8,0xee7e); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x1f2a); /* NPC 3, field +4C. */
}

/* Place 36, 1F2A:EECA. */
void jx_place_enter_036(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 37, 1F2A:EED4. */
void jx_place_enter_037(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 38, 1F2A:EEF4. */
void jx_place_enter_038(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xeede); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0xeede); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x1f2a); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0xeede); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x1f2a); /* NPC 2, field +54. */
    jx_set_word(c,0x89f0,0xeede); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x1f2a); /* NPC 3, field +54. */
    jx_set_word(c,0x8a46,0xeede); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x1f2a); /* NPC 4, field +54. */
    jx_set_word(c,0x8a9c,0xeede); /* NPC 5, field +52. */
    jx_set_word(c,0x8a9e,0x1f2a); /* NPC 5, field +54. */
    jx_set_word(c,0x8af2,0xeede); /* NPC 6, field +52. */
    jx_set_word(c,0x8af4,0x1f2a); /* NPC 6, field +54. */
    jx_set_word(c,0x8b48,0xeede); /* NPC 7, field +52. */
    jx_set_word(c,0x8b4a,0x1f2a); /* NPC 7, field +54. */
    jx_set_word(c,0x8b9e,0xeede); /* NPC 8, field +52. */
    jx_set_word(c,0x8ba0,0x1f2a); /* NPC 8, field +54. */
    jx_set_word(c,0x8bf4,0xeede); /* NPC 9, field +52. */
    jx_set_word(c,0x8bf6,0x1f2a); /* NPC 9, field +54. */
}

/* Place 39, 1F2A:EFA8. */
void jx_place_enter_039(JxEventContext *c) {
    jx_set_word(c,0x6584,0x003f); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0049); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xef76); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
}

/* Place 40, 1F2A:EFCA. */
void jx_place_enter_040(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 41, 1F2A:F000. */
void jx_place_enter_041(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xefd4); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0xefd4); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x1f2a); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0xefea); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x1f2a); /* NPC 2, field +54. */
    jx_set_word(c,0x89f0,0xefea); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x1f2a); /* NPC 3, field +54. */
    jx_set_word(c,0x8a46,0xefea); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x1f2a); /* NPC 4, field +54. */
    jx_set_word(c,0x8a9c,0xefea); /* NPC 5, field +52. */
    jx_set_word(c,0x8a9e,0x1f2a); /* NPC 5, field +54. */
    jx_set_word(c,0x8af2,0xefea); /* NPC 6, field +52. */
    jx_set_word(c,0x8af4,0x1f2a); /* NPC 6, field +54. */
    jx_set_word(c,0x8b48,0xefea); /* NPC 7, field +52. */
    jx_set_word(c,0x8b4a,0x1f2a); /* NPC 7, field +54. */
    jx_set_word(c,0x8b9e,0xefea); /* NPC 8, field +52. */
    jx_set_word(c,0x8ba0,0x1f2a); /* NPC 8, field +54. */
    jx_set_word(c,0x8bf4,0xefea); /* NPC 9, field +52. */
    jx_set_word(c,0x8bf6,0x1f2a); /* NPC 9, field +54. */
    jx_set_word(c,0x8c4a,0xefea); /* NPC 10, field +52. */
    jx_set_word(c,0x8c4c,0x1f2a); /* NPC 10, field +54. */
    jx_set_word(c,0x8ca0,0xefea); /* NPC 11, field +52. */
    jx_set_word(c,0x8ca2,0x1f2a); /* NPC 11, field +54. */
    jx_set_word(c,0x8cf6,0xefea); /* NPC 12, field +52. */
    jx_set_word(c,0x8cf8,0x1f2a); /* NPC 12, field +54. */
}

/* Place 42, 1F2A:F0A6. */
void jx_place_enter_042(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 44, 1F2A:F0CC. */
void jx_place_enter_044(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 45, 1F2A:F0D6. */
void jx_place_enter_045(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 46, 1F2A:F0E0. */
void jx_place_enter_046(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 47, 1F2A:F0EA. */
void jx_place_enter_047(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 48, 1F2A:F0F4. */
void jx_place_enter_048(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 49, 1F2A:F0FE. */
void jx_place_enter_049(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 50, 1F2A:F1A4. */
void jx_place_enter_050(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0017); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0017); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xf108); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x001c); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0012); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xf150); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x001d); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0012); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xf150); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x001e); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x0012); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0xf150); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x1f2a); /* Location trigger 3, callback segment */
}

/* Place 51, 1F2A:F29E. */
void jx_place_enter_051(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xf20e); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
}

/* Place 52, 1F2A:F2D2. */
void jx_place_enter_052(JxEventContext *c) {
    jx_set_word(c,0x6584,0x000e); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0021); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xf2b4); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x000f); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0021); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xf2b4); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0010); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0021); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xf2b4); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_ds(c)[0x6c74]=0x00;
    jx_ds(c)[0x6c75]=0x00;
}

/* Place 53, 1F2A:F344. */
void jx_place_enter_053(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xf32e); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 54, 1F2A:F35A. */
void jx_place_enter_054(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 55, 1F2A:F3F6. */
void jx_place_enter_055(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xf364); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0xf3a4); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
}

/* Place 56, 1F2A:F418. */
void jx_place_enter_056(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 57, 1F2A:F504. */
void jx_place_enter_057(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xf422); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x89e8,0xf440); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x1f2a); /* NPC 3, field +4C. */
    jx_set_word(c,0x6584,0x003e); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0003); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xf460); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x003f); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0003); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xf460); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0040); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0003); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xf460); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x0041); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x0003); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0xf460); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x1f2a); /* Location trigger 3, callback segment */
}

/* Place 58, 1F2A:F586. */
void jx_place_enter_058(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 59, 1F2A:F590. */
void jx_place_enter_059(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 60, 1F2A:F5CC. */
void jx_place_enter_060(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xf59a); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 61, 1F2A:F5E2. */
void jx_place_enter_061(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 62, 1F2A:F5EC. */
void jx_place_enter_062(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 63, 1F2A:F61C. */
void jx_place_enter_063(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xf5f6); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0xf5f6); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
}

/* Place 64, 1F2A:F63E. */
void jx_place_enter_064(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 65, 1F2A:F76E. */
void jx_place_enter_065(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0046); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0031); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xf648); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x000b); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0020); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xf72e); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x000c); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0020); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xf72e); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x000d); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x0020); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0xf72e); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x1f2a); /* Location trigger 3, callback segment */
}

/* Place 66, 1F2A:F7D8. */
void jx_place_enter_066(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 68, 1F2A:F818. */
void jx_place_enter_068(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 69, 1F2A:F8D8. */
void jx_place_enter_069(JxEventContext *c) {
    jx_set_word(c,0x6584,0x000d); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x002e); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xf822); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x000d); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x002f); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xf822); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
}

/* Place 70, 1F2A:FAE2. */
void jx_place_enter_070(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xf9f2); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0xfa0e); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x1f2a); /* NPC 1, field +4C. */
    jx_set_word(c,0x8b9e,0xfaa4); /* NPC 8, field +52. */
    jx_set_word(c,0x8ba0,0x1f2a); /* NPC 8, field +54. */
    jx_set_word(c,0x6584,0x004e); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0021); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xf912); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x004f); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0021); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xf912); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0050); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0021); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0xf912); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x1f2a); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x0051); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x0021); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0xf912); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x1f2a); /* Location trigger 3, callback segment */
    jx_set_word(c,0x65a4,0x0040); /* Location trigger 4, x */
    jx_set_word(c,0x65a6,0x0072); /* Location trigger 4, y */
    jx_set_word(c,0x65a8,0xf950); /* Location trigger 4, callback offset */
    jx_set_word(c,0x65aa,0x1f2a); /* Location trigger 4, callback segment */
    jx_set_word(c,0x65ac,0x0041); /* Location trigger 5, x */
    jx_set_word(c,0x65ae,0x0072); /* Location trigger 5, y */
    jx_set_word(c,0x65b0,0xf950); /* Location trigger 5, callback offset */
    jx_set_word(c,0x65b2,0x1f2a); /* Location trigger 5, callback segment */
    jx_set_word(c,0x65b4,0x0042); /* Location trigger 6, x */
    jx_set_word(c,0x65b6,0x0072); /* Location trigger 6, y */
    jx_set_word(c,0x65b8,0xf950); /* Location trigger 6, callback offset */
    jx_set_word(c,0x65ba,0x1f2a); /* Location trigger 6, callback segment */
}

/* Place 71, 1F2A:FBB8. */
void jx_place_enter_071(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 72, 1F2A:FBC2. */
void jx_place_enter_072(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 73, 1F2A:FBCC. */
void jx_place_enter_073(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 74, 1F2A:FBD6. */
void jx_place_enter_074(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 75, 1F2A:FCD2. */
void jx_place_enter_075(JxEventContext *c) {
    jx_set_word(c,0x899a,0xfbe0); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x1f2a); /* NPC 2, field +54. */
}

/* Place 76, 1F2A:FCE8. */
void jx_place_enter_076(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 77, 1F2A:FCF2. */
void jx_place_enter_077(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 78, 1F2A:FD12. */
void jx_place_enter_078(JxEventContext *c) {
    jx_set_word(c,0x88ee,0xfcfc); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x1f2a); /* NPC 0, field +54. */
}

/* Place 79, 1F2A:FDEC. */
void jx_place_enter_079(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xfd28); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 80, 1F2A:FE02. */
void jx_place_enter_080(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 82, 1F2A:FE4A. */
void jx_place_enter_082(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 83, 1F2A:FEB8. */
void jx_place_enter_083(JxEventContext *c) {
    jx_set_word(c,0x88e6,0xfe54); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x1f2a); /* NPC 0, field +4C. */
}

/* Place 84, 1F2A:FF3C. */
void jx_place_enter_084(JxEventContext *c) {
    jx_set_word(c,0x6584,0x001e); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0010); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0xfece); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x1f2a); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x001f); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0010); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0xfece); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x1f2a); /* Location trigger 1, callback segment */
    jx_set_word(c,0x89f0,0xff0c); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x1f2a); /* NPC 3, field +54. */
    jx_set_word(c,0x8a46,0xff0c); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x1f2a); /* NPC 4, field +54. */
}

/* Place 85, 1F2A:FFA4. */
void jx_place_enter_085(JxEventContext *c) {
    jx_set_word(c,0x899a,0xff8e); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x1f2a); /* NPC 2, field +54. */
}

/* Place 86, 1F2A:FFBA. */
void jx_place_enter_086(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 87, 1F2A:FFC4. */
void jx_place_enter_087(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 88, 1F2A:FFCE. */
void jx_place_enter_088(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 89, 2F28:0000. */
void jx_place_enter_089(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 90, 2F28:000A. */
void jx_place_enter_090(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 91, 2F28:0088. */
void jx_place_enter_091(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0014); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 93, 2F28:0106. */
void jx_place_enter_093(JxEventContext *c) {
    jx_set_word(c,0x89e8,0x00ce); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x2f28); /* NPC 3, field +4C. */
}

/* Place 94, 2F28:017A. */
void jx_place_enter_094(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x011c); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
    jx_set_word(c,0x88ea,0x013a); /* NPC 0, field +4E. */
    jx_set_word(c,0x88ec,0x2f28); /* NPC 0, field +50. */
    jx_set_word(c,0x88c6,0x001c); /* NPC 0, field +2A. */
}

/* Place 95, 2F28:01E2. */
void jx_place_enter_095(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x01a2); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x01a2); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 96, 2F28:0230. */
void jx_place_enter_096(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x0204); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
    jx_set_word(c,0x8944,0x021a); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 97, 2F28:0252. */
void jx_place_enter_097(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 98, 2F28:02E2. */
void jx_place_enter_098(JxEventContext *c) {
    jx_set_word(c,0x893c,0x025c); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x2f28); /* NPC 1, field +4C. */
    jx_set_word(c,0x89f0,0x02be); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x2f28); /* NPC 3, field +54. */
    jx_set_word(c,0x6584,0x0018); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0017); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x0290); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0019); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0017); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x0290); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
}

/* Place 99, 2F28:044E. */
void jx_place_enter_099(JxEventContext *c) {
    jx_set_word(c,0x8a46,0x039e); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x2f28); /* NPC 4, field +54. */
    jx_set_word(c,0x8a3e,0x03d0); /* NPC 4, field +4A. */
    jx_set_word(c,0x8a40,0x2f28); /* NPC 4, field +4C. */
    jx_set_word(c,0x8992,0x0334); /* NPC 2, field +4A. */
    jx_set_word(c,0x8994,0x2f28); /* NPC 2, field +4C. */
}

/* Place 100, 2F28:04FC. */
void jx_place_enter_100(JxEventContext *c) {
    jx_set_word(c,0x893c,0x047c); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x2f28); /* NPC 1, field +4C. */
}

/* Place 101, 2F28:0512. */
void jx_place_enter_101(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 102, 2F28:051C. */
void jx_place_enter_102(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 103, 2F28:05BE. */
void jx_place_enter_103(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0526); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0526); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 104, 2F28:05E0. */
void jx_place_enter_104(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0526); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0526); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 105, 2F28:0602. */
void jx_place_enter_105(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 106, 2F28:0638. */
void jx_place_enter_106(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x060c); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 107, 2F28:069C. */
void jx_place_enter_107(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x064e); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 108, 2F28:06B2. */
void jx_place_enter_108(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 109, 2F28:06BC. */
void jx_place_enter_109(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 110, 2F28:06C6. */
void jx_place_enter_110(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 111, 2F28:06D0. */
void jx_place_enter_111(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 112, 2F28:06DA. */
void jx_place_enter_112(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 113, 2F28:0758. */
void jx_place_enter_113(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x06e4); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x06e4); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0x0724); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x2f28); /* NPC 2, field +54. */
    jx_set_word(c,0x89f0,0x0724); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x2f28); /* NPC 3, field +54. */
}

/* Place 114, 2F28:0926. */
void jx_place_enter_114(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0033); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0050); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x07c6); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0034); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0050); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x07c6); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0035); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0050); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x07c6); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x0036); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x0050); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0x07c6); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x2f28); /* Location trigger 3, callback segment */
    jx_set_word(c,0x8b9e,0x086a); /* NPC 8, field +52. */
    jx_set_word(c,0x8ba0,0x2f28); /* NPC 8, field +54. */
    jx_set_word(c,0x8bf4,0x0792); /* NPC 9, field +52. */
    jx_set_word(c,0x8bf6,0x2f28); /* NPC 9, field +54. */
    jx_set_word(c,0x8c4a,0x0792); /* NPC 10, field +52. */
    jx_set_word(c,0x8c4c,0x2f28); /* NPC 10, field +54. */
}

/* Place 115, 2F28:0A6A. */
void jx_place_enter_115(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x09b4); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0a1c); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 116, 2F28:0A8C. */
void jx_place_enter_116(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 117, 2F28:0A96. */
void jx_place_enter_117(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 119, 2F28:0B0E. */
void jx_place_enter_119(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 120, 2F28:0B18. */
void jx_place_enter_120(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 122, 2F28:0D76. */
void jx_place_enter_122(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0053); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x000e); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x0c78); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0054); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x000e); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x0c78); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0053); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x000f); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x0c78); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x0054); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x000f); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0x0c78); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x2f28); /* Location trigger 3, callback segment */
}

/* Place 123, 2F28:0E32. */
void jx_place_enter_123(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0de0); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0de0); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 124, 2F28:0EA6. */
void jx_place_enter_124(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0e54); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0e54); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0x0e54); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x2f28); /* NPC 2, field +54. */
    jx_set_word(c,0x89f0,0x0e54); /* NPC 3, field +52. */
    jx_set_word(c,0x89f2,0x2f28); /* NPC 3, field +54. */
    jx_set_word(c,0x8a46,0x0e54); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x2f28); /* NPC 4, field +54. */
    jx_set_word(c,0x8a9c,0x0e54); /* NPC 5, field +52. */
    jx_set_word(c,0x8a9e,0x2f28); /* NPC 5, field +54. */
}

/* Place 125, 2F28:0F08. */
void jx_place_enter_125(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0012); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x000b); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x0ef8); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0013); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x000b); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x0ef8); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0014); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x000b); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x0ef8); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
}

/* Place 127, 2F28:0F92. */
void jx_place_enter_127(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0526); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0526); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 128, 2F28:0FB4. */
void jx_place_enter_128(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x0526); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x0526); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 129, 2F28:1098. */
void jx_place_enter_129(JxEventContext *c) {
    jx_set_word(c,0x873e,0x9c3a);
    jx_set_word(c,0x8740,0x2f28);
    jx_set_word(c,0x6584,0x0016); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x005c); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x0fd6); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0017); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x005c); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x0fd6); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0018); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x005c); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x0fd6); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
    jx_set_word(c,0x88e6,0x1074); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 130, 2F28:1102. */
void jx_place_enter_130(JxEventContext *c) {
    jx_set_word(c,0x873e,0x9c3a);
    jx_set_word(c,0x8740,0x2f28);
}

/* Place 131, 2F28:1118. */
void jx_place_enter_131(JxEventContext *c) {
    jx_set_word(c,0x873e,0x9c3a);
    jx_set_word(c,0x8740,0x2f28);
}

/* Place 132, 2F28:112E. */
void jx_place_enter_132(JxEventContext *c) {
    jx_set_word(c,0x873e,0x9c3a);
    jx_set_word(c,0x8740,0x2f28);
}

/* Place 133, 2F28:1206. */
void jx_place_enter_133(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x1144); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
    jx_set_word(c,0x88ea,0x11c4); /* NPC 0, field +4E. */
    jx_set_word(c,0x88ec,0x2f28); /* NPC 0, field +50. */
    jx_set_word(c,0x88c6,0x001e); /* NPC 0, field +2A. */
}

/* Place 134, 2F28:1292. */
void jx_place_enter_134(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x122e); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 135, 2F28:12A8. */
void jx_place_enter_135(JxEventContext *c) {
    jx_set_word(c,0x873e,0x9c3a);
    jx_set_word(c,0x8740,0x2f28);
}

/* Place 136, 2F28:13EE. */
void jx_place_enter_136(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x12be); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0x1356); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x2f28); /* NPC 1, field +4C. */
}

/* Place 137, 2F28:1410. */
void jx_place_enter_137(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 138, 2F28:1538. */
void jx_place_enter_138(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0009); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0041); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x149a); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x88e6,0x141a); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 139, 2F28:1566. */
void jx_place_enter_139(JxEventContext *c) {
    jx_set_word(c,0x585e,0x0000);
    jx_set_word(c,0x93a8,0x0046);
    jx_set_word(c,0x873e,0x9d42);
    jx_set_word(c,0x8740,0x2f28);
}

/* Place 140, 2F28:1664. */
void jx_place_enter_140(JxEventContext *c) {
    jx_set_word(c,0x873e,0x9efe);
    jx_set_word(c,0x8740,0x2f28);
    jx_set_word(c,0x88ee,0x1588); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x893c,0x15f0); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x2f28); /* NPC 1, field +4C. */
    jx_set_word(c,0x89e8,0x15f0); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x2f28); /* NPC 3, field +4C. */
    jx_set_word(c,0x8a94,0x15f0); /* NPC 5, field +4A. */
    jx_set_word(c,0x8a96,0x2f28); /* NPC 5, field +4C. */
}

/* Place 141, 2F28:16AA. */
void jx_place_enter_141(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 142, 2F28:16B4. */
void jx_place_enter_142(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 143, 2F28:16BE. */
void jx_place_enter_143(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 144, 2F28:16C8. */
void jx_place_enter_144(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 145, 2F28:16D2. */
void jx_place_enter_145(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 146, 2F28:16DC. */
void jx_place_enter_146(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 148, 2F28:17B0. */
void jx_place_enter_148(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x1734); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 149, 2F28:17C6. */
void jx_place_enter_149(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 150, 2F28:18A0. */
void jx_place_enter_150(JxEventContext *c) {
    jx_set_word(c,0x893c,0x17d0); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x2f28); /* NPC 1, field +4C. */
}

/* Place 151, 2F28:1984. */
void jx_place_enter_151(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x18b6); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
    jx_set_word(c,0x8940,0x18d6); /* NPC 1, field +4E. */
    jx_set_word(c,0x8942,0x2f28); /* NPC 1, field +50. */
    jx_set_word(c,0x891c,0x001a); /* NPC 1, field +2A. */
    jx_set_word(c,0x89e8,0x18ec); /* NPC 3, field +4A. */
    jx_set_word(c,0x89ea,0x2f28); /* NPC 3, field +4C. */
    jx_set_word(c,0x8a46,0x1942); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x2f28); /* NPC 4, field +54. */
}

/* Place 152, 2F28:19C4. */
void jx_place_enter_152(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 153, 2F28:19CE. */
void jx_place_enter_153(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 155, 2F28:1AC6. */
void jx_place_enter_155(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 156, 2F28:1AF6. */
void jx_place_enter_156(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x1ad0); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 157, 2F28:1B1C. */
void jx_place_enter_157(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x1b0c); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 158, 2F28:1B48. */
void jx_place_enter_158(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x1b32); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 159, 2F28:1B5E. */
void jx_place_enter_159(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 160, 2F28:1C7E. */
void jx_place_enter_160(JxEventContext *c) {
    jx_set_word(c,0x6584,0x000f); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x004b); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x1b68); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0010); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x004b); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x1b68); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0011); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x004b); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x1b68); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
}

/* Place 162, 2F28:1E54. */
void jx_place_enter_162(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 163, 2F28:1F28. */
void jx_place_enter_163(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x1e5e); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 164, 2F28:1F3E. */
void jx_place_enter_164(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 165, 2F28:1F48. */
void jx_place_enter_165(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 166, 2F28:1F86. */
void jx_place_enter_166(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x1f52); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 167, 2F28:1F9C. */
void jx_place_enter_167(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 168, 2F28:1FA6. */
void jx_place_enter_168(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 169, 2F28:1FB0. */
void jx_place_enter_169(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 170, 2F28:1FBA. */
void jx_place_enter_170(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 174, 2F28:21BE. */
void jx_place_enter_174(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 175, 2F28:21DE. */
void jx_place_enter_175(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x21c8); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 176, 2F28:220A. */
void jx_place_enter_176(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x21f4); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 177, 2F28:2246. */
void jx_place_enter_177(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x2220); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 178, 2F28:225C. */
void jx_place_enter_178(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 179, 2F28:2266. */
void jx_place_enter_179(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 180, 2F28:2270. */
void jx_place_enter_180(JxEventContext *c) {
    jx_set_word(c,0x889a,0x0006);
}

/* Place 181, 2F28:2280. */
void jx_place_enter_181(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 182, 2F28:22FA. */
void jx_place_enter_182(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x228a); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 183, 2F28:2310. */
void jx_place_enter_183(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 184, 2F28:231A. */
void jx_place_enter_184(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 186, 2F28:2450. */
void jx_place_enter_186(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x238a); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x88ea,0x23d2); /* NPC 0, field +4E. */
    jx_set_word(c,0x88ec,0x2f28); /* NPC 0, field +50. */
    jx_set_word(c,0x88c6,0x0023); /* NPC 0, field +2A. */
}

/* Place 187, 2F28:2478. */
void jx_place_enter_187(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 188, 2F28:24E0. */
void jx_place_enter_188(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x2482); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 189, 2F28:255A. */
void jx_place_enter_189(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x24f6); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x24f6); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0x2544); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x2f28); /* NPC 2, field +54. */
}

/* Place 190, 2F28:2588. */
void jx_place_enter_190(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 191, 2F28:26DC. */
void jx_place_enter_191(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x2592); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x2592); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 192, 2F28:271C. */
void jx_place_enter_192(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0030); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0013); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x26fe); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0031); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x0013); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x26fe); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0032); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x0013); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x26fe); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
}

/* Place 193, 2F28:2CAA. */
void jx_place_enter_193(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0018); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x000e); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x2aee); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x8944,0x2b2a); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
    jx_set_word(c,0x899a,0x2b58); /* NPC 2, field +52. */
    jx_set_word(c,0x899c,0x2f28); /* NPC 2, field +54. */
}

/* Place 194, 2F28:2D86. */
void jx_place_enter_194(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x2ce4); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 195, 2F28:2E6A. */
void jx_place_enter_195(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x2d9c); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
    jx_set_word(c,0x893c,0x2e34); /* NPC 1, field +4A. */
    jx_set_word(c,0x893e,0x2f28); /* NPC 1, field +4C. */
}

/* Place 196, 2F28:2E8C. */
void jx_place_enter_196(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 197, 2F28:2E96. */
void jx_place_enter_197(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 198, 2F28:2EA0. */
void jx_place_enter_198(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 199, 2F28:2EAA. */
void jx_place_enter_199(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 200, 2F28:2F82. */
void jx_place_enter_200(JxEventContext *c) {
    jx_set_word(c,0x8a9c,0x2eb4); /* NPC 5, field +52. */
    jx_set_word(c,0x8a9e,0x2f28); /* NPC 5, field +54. */
    jx_set_word(c,0x6584,0x003a); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x004e); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x2f44); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x003b); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x004e); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x2f44); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
}

/* Place 201, 2F28:2FD2. */
void jx_place_enter_201(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x2fc8); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
}

/* Place 202, 2F28:2FE8. */
void jx_place_enter_202(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 203, 2F28:2FF2. */
void jx_place_enter_203(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 204, 2F28:3030. */
void jx_place_enter_204(JxEventContext *c) {
    jx_set_word(c,0x88ea,0x2ffc); /* NPC 0, field +4E. */
    jx_set_word(c,0x88ec,0x2f28); /* NPC 0, field +50. */
    jx_set_word(c,0x88c6,0x0022); /* NPC 0, field +2A. */
}

/* Place 205, 2F28:31B8. */
void jx_place_enter_205(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0066); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x0039); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x304c); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x8a46,0x308a); /* NPC 4, field +52. */
    jx_set_word(c,0x8a48,0x2f28); /* NPC 4, field +54. */
    jx_set_word(c,0x8a9c,0x308a); /* NPC 5, field +52. */
    jx_set_word(c,0x8a9e,0x2f28); /* NPC 5, field +54. */
    jx_set_word(c,0x8aee,0x30d0); /* NPC 6, field +4E. */
    jx_set_word(c,0x8af0,0x2f28); /* NPC 6, field +50. */
    jx_set_word(c,0x8aca,0x0022); /* NPC 6, field +2A. */
}

/* Place 206, 2F28:3204. */
void jx_place_enter_206(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 207, 2F28:323C. */
void jx_place_enter_207(JxEventContext *c) {
    jx_set_word(c,0x88e6,0x320e); /* NPC 0, field +4A. */
    jx_set_word(c,0x88e8,0x2f28); /* NPC 0, field +4C. */
}

/* Place 208, 2F28:3252. */
void jx_place_enter_208(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 209, 2F28:325C. */
void jx_place_enter_209(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 210, 2F28:32A2. */
void jx_place_enter_210(JxEventContext *c) {
    jx_set_word(c,0x88ee,0x3266); /* NPC 0, field +52. */
    jx_set_word(c,0x88f0,0x2f28); /* NPC 0, field +54. */
    jx_set_word(c,0x8944,0x3266); /* NPC 1, field +52. */
    jx_set_word(c,0x8946,0x2f28); /* NPC 1, field +54. */
}

/* Place 211, 2F28:32C4. */
void jx_place_enter_211(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 212, 2F28:32CE. */
void jx_place_enter_212(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 214, 2F28:350A. */
void jx_place_enter_214(JxEventContext *c) {
    jx_set_word(c,0x6584,0x0035); /* Location trigger 0, x */
    jx_set_word(c,0x6586,0x002a); /* Location trigger 0, y */
    jx_set_word(c,0x6588,0x34c6); /* Location trigger 0, callback offset */
    jx_set_word(c,0x658a,0x2f28); /* Location trigger 0, callback segment */
    jx_set_word(c,0x658c,0x0035); /* Location trigger 1, x */
    jx_set_word(c,0x658e,0x002b); /* Location trigger 1, y */
    jx_set_word(c,0x6590,0x34c6); /* Location trigger 1, callback offset */
    jx_set_word(c,0x6592,0x2f28); /* Location trigger 1, callback segment */
    jx_set_word(c,0x6594,0x0036); /* Location trigger 2, x */
    jx_set_word(c,0x6596,0x002b); /* Location trigger 2, y */
    jx_set_word(c,0x6598,0x34c6); /* Location trigger 2, callback offset */
    jx_set_word(c,0x659a,0x2f28); /* Location trigger 2, callback segment */
    jx_set_word(c,0x659c,0x0037); /* Location trigger 3, x */
    jx_set_word(c,0x659e,0x002b); /* Location trigger 3, y */
    jx_set_word(c,0x65a0,0x34c6); /* Location trigger 3, callback offset */
    jx_set_word(c,0x65a2,0x2f28); /* Location trigger 3, callback segment */
    jx_set_word(c,0x65a4,0x0038); /* Location trigger 4, x */
    jx_set_word(c,0x65a6,0x002b); /* Location trigger 4, y */
    jx_set_word(c,0x65a8,0x34c6); /* Location trigger 4, callback offset */
    jx_set_word(c,0x65aa,0x2f28); /* Location trigger 4, callback segment */
    jx_set_word(c,0x65ac,0x0039); /* Location trigger 5, x */
    jx_set_word(c,0x65ae,0x002b); /* Location trigger 5, y */
    jx_set_word(c,0x65b0,0x34c6); /* Location trigger 5, callback offset */
    jx_set_word(c,0x65b2,0x2f28); /* Location trigger 5, callback segment */
    jx_set_word(c,0x65b4,0x0039); /* Location trigger 6, x */
    jx_set_word(c,0x65b6,0x002a); /* Location trigger 6, y */
    jx_set_word(c,0x65b8,0x34c6); /* Location trigger 6, callback offset */
    jx_set_word(c,0x65ba,0x2f28); /* Location trigger 6, callback segment */
    jx_set_word(c,0x8a3e,0x34f4); /* NPC 4, field +4A. */
    jx_set_word(c,0x8a40,0x2f28); /* NPC 4, field +4C. */
}

/* Place 215, 2F28:35C8. */
void jx_place_enter_215(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 216, 2F28:35D2. */
void jx_place_enter_216(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 217, 2F28:35DC. */
void jx_place_enter_217(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 218, 2F28:35E6. */
void jx_place_enter_218(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}

/* Place 219, 2F28:35F0. */
void jx_place_enter_219(JxEventContext *c) {
    (void)c; /* Original function only has its stack frame and return. */
}
