#include "event_engine.h"

/* 1F2A:DDCC: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_ddcc(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xb6c1]==0) (void)JX_CALL0(c,0x17d2,0x0012);
}

/* 1F2A:DFA2: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_dfa2(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xb739]==0) (void)JX_CALL0(c,0x17d2,0x001e);
}

/* 1F2A:E350: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_e350(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0x67c5]!=0) (void)JX_CALL0(c,0x17d2,0x007e);
}

/* 1F2A:E5D8: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_e5d8(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0x68b3]!=0) (void)JX_CALL0(c,0x17d2,0x00c6);
}

/* 1F2A:E796: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_e796(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0x6903]!=0) (void)JX_CALL0(c,0x17d2,0x00f0);
}

/* 1F2A:EB3E: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_eb3e(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xba27]==0) (void)JX_CALL0(c,0x17d2,0x010e);
}

/* 1F2A:EDAA: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_edaa(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xba63]==0) (void)JX_CALL0(c,0x17d2,0x0120);
}

/* 1F2A:F2B4: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_f2b4(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xbcba]==0) (void)JX_CALL0(c,0x17d2,0x0138);
}

/* 1F2A:F422: signed-byte story flag, original conditional preserved. */
void jx_callback_1f2a_f422(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xbd34]>=5) jx_ds(c)[0xbd34]=6;
}

/* 2F28:011C: signed-byte story flag, original conditional preserved. */
void jx_callback_2f28_011c(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xc16f]<=4) jx_ds(c)[0xc16f]=5;
}

/* 2F28:19D8: signed-byte story flag, original conditional preserved. */
void jx_callback_2f28_19d8(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xc8ac]==0) (void)JX_CALL0(c,0x17d2,0x01aa);
}

/* 2F28:26FE: signed-byte story flag, original conditional preserved. */
void jx_callback_2f28_26fe(JxEventContext *c) {
    if ((int8_t)jx_ds(c)[0xcd23]==0) (void)JX_CALL0(c,0x17d2,0x01ce);
}

/* 2F28:320E: enable seven consecutive story flags. */
void jx_callback_2f28_320e(JxEventContext *c) {
    for (uint16_t flag=0x7f6e;flag<=0x7f74;flag++) jx_ds(c)[flag]=1;
}
