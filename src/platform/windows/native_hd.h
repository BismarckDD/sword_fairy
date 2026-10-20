#ifndef JX_NATIVE_HD_H
#define JX_NATIVE_HD_H
#include "video_model.h"
typedef struct JxHd JxHd;
/* Optional native presentation adapter; never writes guest game state. */
JxHd *jx_hd_open(const char *directory);
void jx_hd_close(JxHd *);
void jx_hd_walk_slot(JxHd *,unsigned slot,unsigned resource,unsigned base);
void jx_hd_walk_source(JxHd *,unsigned kind,unsigned index);
unsigned jx_hd_walk_resources(const JxHd *);
unsigned jx_hd_walk_cache_bytes(const JxHd *);
unsigned jx_hd_walk_hits(const JxHd *);
unsigned jx_hd_asset_count(const JxHd *);
unsigned jx_hd_hits(const JxHd *);
unsigned jx_hd_tile_hits(const JxHd *);
unsigned jx_hd_animation_hits(const JxHd *);
void jx_hd_reset(JxHd *);
void jx_hd_read(JxHd *,unsigned address);
void jx_hd_write(JxHd *,const JxVideoModel *,unsigned address);
void jx_hd_blit(JxHd *,JxEventContext *,const JxVideoModel *,const uint16_t *,int);
/* Plane-major world tiles (1832:13B0), width rounded down to 8 pixels. */
void jx_hd_blit_planes(JxHd *,JxEventContext *,const JxVideoModel *,const uint16_t *);
/* Observe the 16x16 plane-major world workspace before CPU tile copies. */
void jx_hd_tile_copy(JxHd *,JxEventContext *,const uint16_t *,int transparent);
void jx_hd_tile_capture(JxHd *,JxEventContext *,const JxVideoModel *,const uint16_t *);
void jx_hd_capture(JxHd *,JxEventContext *,const uint16_t *);
/* Observe dialogue/choice/menu workspace captures before their shaded blit. */
void jx_hd_dialogue_capture(JxHd *,JxEventContext *,const JxVideoModel *,const uint8_t *,const uint16_t *);
/* Full-screen UI uses a centered 640x400 layout with original 32px glyphs.
 * Nested screens share one scale; plain mode supports direct VGA browsers. */
void jx_hd_ui_begin(JxHd *,const JxVideoModel *,const uint8_t *,int plain);
void jx_hd_ui_end(JxHd *);
/* Returns a 1280x800 top-down 0x00RRGGBB framebuffer owned by the adapter. */
const uint32_t *jx_hd_render(JxHd *,const JxVideoModel *,const uint8_t *palette);
int jx_hd_export_bmp(JxHd *,const JxVideoModel *,const uint8_t *,const char *);
/* Indexed Scale2x twice, retaining palette indices and transparency masks. */
void jx_hd_scale4(const uint8_t *,unsigned,unsigned,uint8_t *,uint8_t *);
#endif
