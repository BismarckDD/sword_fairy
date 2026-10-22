#include "event_engine.h"

/* 1F2A:5F94, display defaults after switching to unchained VGA. */
void jx_graphics_initialize(JxEventContext *c) {
    (void)JX_CALL(c,0x1832,0x17ae);
    (void)JX_CALL(c,0x2f28,0xa572,0x61de,c->data_segment,0,160);
    (void)JX_CALL(c,0x1f2a,0x06f2,95,0,1);
    jx_set_word(c,0x585e,0);jx_set_word(c,0x5860,1);jx_set_word(c,0x6658,15);
}
static void boot_text(JxEventContext *c,uint16_t text) {
    (void)JX_CALL(c,0x1f2a,0x0728,100,100,text,c->data_segment);
}
static void boot_clear(JxEventContext *c) {
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
}
static void boot_palette(JxEventContext *c) {
    (void)JX_CALL(c,0x150f,0x0006,0x61de,c->data_segment,0,160);
}
static void boot_font(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
    (void)JX_CALL(c,0x1f2a,0x0718,176);
}
/* 1F2A:6B38. Shutdown/exit callbacks terminate the host execution when
 * appropriate. If a diagnostic host lets them return, preserve the original
 * fall-through rather than inventing an early return. */
void jx_game_initialize(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x5528);
    (void)JX_CALL(c,0x1f2a,0x5436);
    (void)JX_CALL(c,0x1f2a,0x5b56);
    (void)JX_CALL(c,0x1f2a,0x5f60);
    (void)JX_CALL(c,0x1f2a,0x5f78);
    if(jx_word(c,0x31b6)!=0)(void)JX_CALL(c,0x1f2a,0x14ec);
    (void)JX_CALL(c,0x1f2a,0x5f94);
    if((uint16_t)JX_CALL(c,0x1832,0x2098,0x6604,c->data_segment)!=0) {
        boot_text(c,0x0492);(void)JX_CALL(c,0x1f2a,0x7272);boot_clear(c);
        if((uint16_t)JX_CALL(c,0x2f28,0x866a)!=0) {
            boot_text(c,0x04a1);(void)JX_CALL(c,0x1f2a,0x2dbc,30);
            (void)JX_CALL(c,0x1f2a,0xb196);
        }
    }
    if((uint16_t)JX_CALL(c,0x2f28,0x866a)!=0) {
        boot_text(c,0x04ae);(void)JX_CALL(c,0x1f2a,0x7272);boot_clear(c);
        if((uint16_t)JX_CALL(c,0x2f28,0x866a)!=0) {
            boot_text(c,0x04bd);(void)JX_CALL(c,0x1f2a,0x2dbc,30);
            (void)JX_CALL(c,0x1f2a,0xb196);
        }
    }
    (void)JX_CALL(c,0x1832,0x21d8,0x932c,c->data_segment);
    (void)JX_CALL(c,0x1f2a,0x60ac);
    for(;;) {
        (void)JX_CALL(c,0x1f2a,0x58fa,0x04ca,c->data_segment);
        if((uint16_t)JX_CALL(c,0x1f2a,0x7baa,200,50,0x04db,c->data_segment,0x04d4,c->data_segment)==0) {
            boot_clear(c);boot_palette(c);boot_font(c);
            (void)JX_CALL(c,0x1f2a,0x629e);
            (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
            break;
        }
        boot_font(c);boot_clear(c);boot_palette(c);
        /* Escape cancels loading and returns to the title, rather than
         * immediately reopening the save browser. */
        if((uint16_t)JX_CALL(c,0x1f2a,0xa2f0)==0)break;
        /* The browser was opened after clearing the title. Reload its
         * full background and palette before displaying the choices again. */
        (void)JX_CALL(c,0x2f28,0x6dfe,0x043b,c->data_segment);
    }
    (void)JX_CALL(c,0x1f2a,0x58fa,0x04e2,c->data_segment);
}
/* 1F2A:B1FA. The CRT supplies argc/argv/environment words but this game
 * entry never reads them. Main loop normally exits through termination. */
uint16_t jx_game_entry(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x6b38);
    (void)JX_CALL(c,0x1f2a,0xae40);
    (void)JX_CALL(c,0x1f2a,0xb196);
    return 0;
}
