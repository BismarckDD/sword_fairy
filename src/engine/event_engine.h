#ifndef JX_EVENT_ENGINE_H
#define JX_EVENT_ENGINE_H
#include <stdint.h>
#include <stddef.h>

/* Reconstructed event ABI. Memory holds the original 20-bit address space.
 * Unknown engine functions remain required dependencies, never no-op stubs.
 * args are 16-bit words in C argument order; result is original DX:AX.
 * The callback may change memory. Event code re-reads globals after calls.
 */
typedef uint32_t (*JxEngineCall)(void *opaque, uint16_t segment,
    uint16_t offset, const uint16_t *args, size_t count);
typedef struct {
    uint8_t *memory;
    uint16_t data_segment;
    JxEngineCall call;
    void *opaque;
    /* Optional native save browser: 0=load, 1=save, 2=capture clean scene.
     * Returns zero-based slot, -1=cancel, -2=use original menu. */
    int (*save_browser)(void *,int);
    void *save_browser_opaque;
} JxEventContext;
void jx_callback_1512_000c(JxEventContext *);
void jx_callback_1512_016a(JxEventContext *);

typedef struct {uint16_t offset,segment;} JxFarPointer;
typedef struct {
    void *opaque;
    uint16_t (*get)(void *);
    void (*unget)(void *,uint16_t);
} JxScanInput;
uint16_t jx_scan_decimal16(JxEventContext *,const JxScanInput *,uint16_t *);
uint16_t jx_scan_token(JxEventContext *,const JxScanInput *,uint8_t *,uint16_t);
void jx_world_tick(JxEventContext *);
void jx_palette_rotate_range(JxEventContext *,uint16_t,uint16_t,uint16_t);
void jx_world_palette_ranges(JxEventContext *);
void jx_world_periodic_sounds(JxEventContext *);
void jx_world_check_disc(JxEventContext *);
void jx_world_cd_update(JxEventContext *);
uint16_t jx_keyboard_randomize_direction(JxEventContext *,uint16_t);
uint16_t jx_keyboard_read_direction(JxEventContext *);
/* A host must halt/dispatch the original virtual INT0 on any error;
 * these values describe control flow, not the original undefined return AX. */
typedef enum {JX_WORLD_UPDATE_RETURNED=0,JX_WORLD_IDLE_DIVIDE_ERROR=1,
    JX_WORLD_BACKGROUND_DIVIDE_ERROR=2,
    JX_WORLD_NPC_IDLE_DIVIDE_ERROR=3,
    JX_WORLD_NPC_ACTIVE_DIVIDE_ERROR=4,
    JX_WORLD_PLAYER_DIVIDE_ERROR=5,
    JX_WORLD_COLLISION_IDLE_DIVIDE_ERROR=6,
    JX_WORLD_COLLISION_ACTIVE_DIVIDE_ERROR=7,
    JX_WORLD_NPC_IDLE_FRAME_DIVIDE_ERROR=8,
    JX_WORLD_NPC_ACTIVE_FRAME_DIVIDE_ERROR=9,
    JX_WORLD_SCENE_IDLE_DIVIDE_ERROR=10,
    JX_WORLD_SCENE_ACTIVE_DIVIDE_ERROR=11} JxWorldUpdateFlow;
JxWorldUpdateFlow jx_scene_cell_layers_world(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
JxWorldUpdateFlow jx_movement_classify_cells(JxEventContext *,uint16_t,uint16_t,JxFarPointer,uint16_t *);
JxWorldUpdateFlow jx_scene_draw_tile_world(JxEventContext *,int16_t,int16_t,JxFarPointer);
JxWorldUpdateFlow jx_scene_redraw_world(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_party_redraw_trail_world(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_party_redraw_page_world(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_party_scroll_page_world(JxEventContext *,uint16_t,JxFarPointer);
JxWorldUpdateFlow jx_party_move_cycle_world(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
JxWorldUpdateFlow jx_npc_redraw_background_world(JxEventContext *,int16_t,JxFarPointer);
JxWorldUpdateFlow jx_npc_idle_update(JxEventContext *,uint16_t);
JxWorldUpdateFlow jx_npc_idle_update_cells(JxEventContext *,uint16_t,JxFarPointer);
JxWorldUpdateFlow jx_npc_active_update(JxEventContext *,uint16_t);
JxWorldUpdateFlow jx_world_player_move(JxEventContext *);
JxWorldUpdateFlow jx_world_player_move_collision(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_player_cell_status_collision(JxEventContext *,uint16_t,uint16_t,JxFarPointer,uint16_t *);
JxWorldUpdateFlow jx_scene_collision_layers(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_player_cell_status(JxEventContext *,uint16_t,uint16_t);
JxWorldUpdateFlow jx_world_npc_update(JxEventContext *);
JxWorldUpdateFlow jx_world_npc_update_actions(JxEventContext *);
JxWorldUpdateFlow jx_world_npc_update_cells(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_world_idle_redraw(JxEventContext *);
JxWorldUpdateFlow jx_world_background_cycle(JxEventContext *);
JxWorldUpdateFlow jx_world_idle_redraw_cells(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_world_background_cycle_cells(JxEventContext *,JxFarPointer);
JxWorldUpdateFlow jx_world_tick_state(JxEventContext *);
JxWorldUpdateFlow jx_world_tick_collision(JxEventContext *,JxFarPointer);
/* DOS multiplex INT 2F AX=1510: drive in CX, request at ES:BX.
 * Service may mutate the packet and game memory; AX is explicitly returned. */
typedef uint16_t (*JxCdRequestService)(void *,uint16_t,JxFarPointer);
typedef struct {void *opaque;JxCdRequestService request;} JxCdDevice;
uint16_t jx_cd_send_request(const JxCdDevice *,uint16_t,JxFarPointer);
uint16_t jx_cd_stop(JxEventContext *,const JxCdDevice *,uint8_t);
uint16_t jx_cd_resume(JxEventContext *,const JxCdDevice *,uint8_t);
uint16_t jx_cd_play(JxEventContext *,const JxCdDevice *,uint8_t,uint32_t,uint32_t,uint8_t);
uint16_t jx_cd_ioctl_input(JxEventContext *,const JxCdDevice *,uint8_t,JxFarPointer,uint8_t,uint8_t);
uint16_t jx_cd_audio_position(JxEventContext *,const JxCdDevice *,uint8_t,JxFarPointer);
uint8_t jx_bcd_to_binary(uint8_t);
uint32_t jx_cd_msf_to_frames(uint32_t);
uint32_t jx_cd_frames_to_msf(uint32_t);
uint16_t jx_cd_disc_info(JxEventContext *,const JxCdDevice *,uint8_t,JxFarPointer);
/* Table needs 4*(last track+1) bytes, private scratch needs 7 bytes. */
uint16_t jx_cd_track_table(JxEventContext *,const JxCdDevice *,uint8_t,JxFarPointer,JxFarPointer);
/* Valid original CD track range is 1..99. Private table is 400 bytes and
 * scratch is 7 bytes, disjoint from live data; preserve initial bytes on errors. */
uint16_t jx_cd_play_track(JxEventContext *,const JxCdDevice *,uint8_t,uint8_t,uint32_t,uint32_t,JxFarPointer,JxFarPointer);
uint16_t jx_cd_play_seconds(JxEventContext *,const JxCdDevice *,uint8_t,uint16_t,uint16_t,JxFarPointer,JxFarPointer);
uint16_t jx_cd_audio_status(JxEventContext *,const JxCdDevice *,uint8_t,JxFarPointer);
uint16_t jx_cd_current_status(JxEventContext *,const JxCdDevice *,JxFarPointer);
uint16_t jx_cd_current_position(JxEventContext *,const JxCdDevice *,JxFarPointer);
uint16_t jx_cd_stop_current(JxEventContext *,const JxCdDevice *);
uint16_t jx_cd_stop_if_active(JxEventContext *,const JxCdDevice *,JxFarPointer);
uint16_t jx_cd_resume_if_paused(JxEventContext *,const JxCdDevice *,JxFarPointer);
uint16_t jx_cd_current_disc(JxEventContext *,const JxCdDevice *,JxFarPointer);
typedef struct {uint16_t count,first_drive;} JxCdInstallation;
typedef void (*JxCdQueryService)(void *,JxCdInstallation *);
typedef uint8_t (*JxCdListService)(void *,JxFarPointer);
typedef struct {void *opaque;JxCdQueryService query;JxCdListService list;} JxCdDiscovery;
uint16_t jx_cd_first_drive(const JxCdDiscovery *);
uint16_t jx_cd_drive_count(const JxCdDiscovery *);
uint16_t jx_cd_device_list(const JxCdDiscovery *,JxFarPointer);
uint16_t jx_cd_initialize(JxEventContext *,const JxCdDiscovery *,JxFarPointer,JxFarPointer);
uint16_t jx_cd_initialize_current(JxEventContext *,const JxCdDiscovery *,JxFarPointer);
/* EMS INT67 service boundary. Only defined inputs are supplied; unused
 * BX/DX/physical-page inputs are zero. Host returns AH status plus BX/DX
 * outputs and preserves the caller's segment/pointer registers. */
typedef uint8_t (*JxEmsService)(void *,uint8_t function,uint8_t physical_page,
    uint16_t bx_input,uint16_t dx_input,uint16_t *bx_output,uint16_t *dx_output);
typedef struct {void *opaque;JxEmsService service;} JxEmsDevice;
uint16_t jx_ems_status(const JxEmsDevice *);
uint16_t jx_ems_frame(JxEventContext *,const JxEmsDevice *,JxFarPointer);
uint16_t jx_ems_page_counts(JxEventContext *,const JxEmsDevice *,JxFarPointer,JxFarPointer);
uint16_t jx_ems_allocate(JxEventContext *,const JxEmsDevice *,uint16_t,JxFarPointer);
uint16_t jx_ems_map(const JxEmsDevice *,uint16_t,uint16_t,uint16_t);
uint16_t jx_ems_release(const JxEmsDevice *,uint16_t);
uint32_t jx_memory_copy(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
int16_t jx_memory_compare(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
int jx_divide_signed32(uint32_t,uint32_t,uint32_t *);
uint32_t jx_shift_left32(uint32_t,uint16_t);
uint32_t jx_shift_right_unsigned32(uint32_t,uint16_t);
uint32_t jx_shift_right_signed32(uint32_t,uint16_t);
typedef struct {
    void *opaque;
    uint8_t (*read)(void *,uint16_t);
    void (*write)(void *,uint16_t,uint8_t);
} JxPortDevice;
uint16_t jx_port_read_byte(const JxPortDevice *,uint16_t);
uint16_t jx_port_write_byte(const JxPortDevice *,uint16_t,uint16_t);
int jx_remainder_signed32(uint32_t,uint32_t,uint32_t *);
uint16_t jx_calendar_is_daylight(JxEventContext *,JxFarPointer);
void jx_timezone_ensure_initialized(JxEventContext *);
uint16_t jx_ftime(JxEventContext *,JxFarPointer,JxFarPointer);
uint32_t jx_string_copy_bounded(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
int16_t jx_string_compare_bounded(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
uint32_t jx_atol32(JxEventContext *,JxFarPointer);
uint32_t jx_environment_get(JxEventContext *,JxFarPointer);
void jx_timezone_initialize(JxEventContext *);
uint32_t jx_calendar_to_seconds(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer);
uint32_t jx_memory_fill(JxEventContext *,JxFarPointer,uint16_t,uint16_t);
void jx_memory_copy_transparent(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,uint16_t);
uint16_t jx_inventory_has(JxEventContext *,uint16_t);
void jx_inventory_compact(JxEventContext *);
void jx_inventory_remove(JxEventContext *,uint16_t,uint16_t);
void jx_actor_skills_compact(JxEventContext *,uint16_t);
void jx_actor_skill_remove(JxEventContext *,int16_t,uint16_t);
uint16_t jx_absolute_word(uint16_t);
void jx_swap_words(JxEventContext *,JxFarPointer,JxFarPointer);
/* Signed endpoints with original 16-bit loop wrap. A terminal coordinate
 * of 32767 can make these original loops nonterminating; no clipping. */
void jx_draw_vertical(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t);
void jx_draw_line(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
uint16_t jx_random_seed(JxEventContext *,uint16_t);
uint16_t jx_random_next(JxEventContext *);
uint32_t jx_multiply_u32(uint32_t,uint32_t);
uint32_t jx_string_copy(JxEventContext *,JxFarPointer,JxFarPointer);
uint32_t jx_string_append(JxEventContext *,JxFarPointer,JxFarPointer);
/* sprintf entry with explicit original 16-bit vararg memory. Formatter and
 * overflow writer remain required engine dependencies. Shared CRT stream
 * state means this entry is not reentrant. */
/* Internal formatter routines: explicit state replaces BP-relative locals.
 * These are near helpers, not additional independent game entry points. */
uint16_t jx_format_put_byte(JxEventContext *,JxFarPointer,uint8_t);
uint16_t jx_format_repeat(JxEventContext *,JxFarPointer,uint8_t,uint16_t,uint16_t);
uint16_t jx_format_span(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,uint16_t);
typedef struct {JxFarPointer start;uint16_t count;} JxFormatDigits;
/* radix must be 8, 10 or 16; end points to the last output byte. */
JxFormatDigits jx_format_digits(JxEventContext *,JxFarPointer,uint32_t,uint16_t,uint16_t,uint8_t);
/* Formatter workspace: 532 private bytes, disjoint from all inputs/outputs.
 * Precision/output must fit the original 512-byte numeric scratch region.
 * Dynamic floating converters and overflow writer remain required callbacks. */
uint16_t jx_format_words(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer,JxFarPointer);
/* CRT streams are addressed in DS; original far stream segment is ignored.
 * Character is a private two-byte argument cell, live across write callbacks. */
/* DOS seek service preserves BX/CX and returns AX/DX/carry explicitly. */
typedef struct {uint16_t ax,dx;uint8_t carry;} JxDosSeekResult;
typedef void (*JxDosSeekService)(void *,uint16_t handle,uint8_t origin,uint32_t offset,JxDosSeekResult *result);
typedef struct {uint16_t ax;uint8_t carry;} JxDosWriteResult;
typedef void (*JxDosWriteService)(void *,uint16_t,JxFarPointer,uint16_t,JxDosWriteResult *);
/* Internal binary branch only: caller has validated handle, invoked optional
 * hook and performed append seek. DOS service preserves BX/DX/segments. */
typedef enum {JX_WRITE_RETURNED=0,JX_WRITE_STACK_FAILURE=1} JxWriteFlow;
typedef struct {JxWriteFlow flow;uint16_t value;} JxFileWriteResult;
typedef enum {JX_STACK_RETURNED=0,JX_STACK_HANDLER=1,JX_STACK_STARTUP_ERROR=2} JxStackProbeFlow;
typedef struct {
    JxStackProbeFlow flow;
    uint16_t ax,bx,cx,dx,sp,flags;
    JxFarPointer target;
} JxStackProbeResult;
JxStackProbeResult jx_stack_probe(JxEventContext *,JxFarPointer,uint16_t,uint16_t);
/* Optional original near hook; preserve registers, including BX handle. */
typedef void (*JxWriteHook)(void *,uint16_t hook_offset,uint16_t handle);
/* Private 512-byte scratch; available_stack is the original 4A7A result.
 * STACK_FAILURE must stop the caller; it is not a normal write error. */
JxFileWriteResult jx_file_write(JxEventContext *,uint16_t,JxFarPointer,uint16_t,
    JxFarPointer,uint16_t,JxDosWriteService,JxDosSeekService,JxWriteHook,void *);
uint16_t jx_file_write_binary(JxEventContext *,uint16_t,JxFarPointer,uint16_t,JxDosWriteService,void *);
typedef struct {JxFarPointer pointer;uint16_t rounded_size;uint8_t failed;} JxHeapBlockResult;
typedef struct {uint16_t segment;uint8_t carry;} JxDosAllocateResult;
typedef void (*JxDosAllocateService)(void *,uint16_t paragraphs,JxDosAllocateResult *);
typedef struct {uint16_t ax,bx;uint8_t carry;} JxDosParagraphResult;
typedef void (*JxDosParagraphService)(void *,uint16_t,JxDosParagraphResult *);
uint16_t jx_dos_allocate_paragraphs(JxEventContext *,uint16_t,JxFarPointer,JxDosParagraphService,void *);
typedef struct {uint16_t masked,attributes;} JxPermissionMaskResult;
JxPermissionMaskResult jx_file_mask_permissions(JxEventContext *,uint16_t);
typedef struct {JxFarPointer heap;uint8_t failed;} JxHeapGrowResult;
JxHeapGrowResult jx_heap_grow(JxEventContext *,JxFarPointer,uint16_t,JxDosAllocateService,void *);
typedef uint8_t (*JxDosResizeService)(void *,uint16_t segment,uint16_t paragraphs);
uint32_t jx_malloc(JxEventContext *,uint16_t,JxDosAllocateService,JxDosResizeService,void *);
uint16_t jx_heap_tail(JxEventContext *,JxFarPointer);
uint8_t jx_heap_resize(JxEventContext *,JxFarPointer,uint16_t,JxDosResizeService,void *);
uint8_t jx_heap_expand(JxEventContext *,JxFarPointer,uint16_t,JxDosResizeService,void *);
void jx_heap_initialize(JxEventContext *,JxFarPointer,uint16_t);
void jx_heap_link(JxEventContext *,JxFarPointer,JxFarPointer);
JxHeapBlockResult jx_heap_take_block(JxEventContext *,JxFarPointer,uint16_t);
uint16_t jx_stack_available(JxEventContext *,uint16_t caller_sp);
uint16_t jx_dos_error(JxEventContext *,uint16_t);
uint32_t jx_file_seek(JxEventContext *,uint16_t,uint32_t,uint16_t,JxDosSeekService,void *);
typedef struct {
    void *opaque;
    JxDosAllocateService allocate;
    JxDosResizeService resize;
    JxDosWriteService write;
    JxDosSeekService seek;
    JxWriteHook hook;
    JxFarPointer write_scratch;
    uint16_t available_stack;
} JxCrtServices;
JxFileWriteResult jx_format_words_dos(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer,JxFarPointer,const JxCrtServices *);
JxFileWriteResult jx_stream_overflow_dos(JxEventContext *,uint16_t,JxFarPointer,const JxCrtServices *);
void jx_stream_allocate_buffer_dos(JxEventContext *,uint16_t,JxDosAllocateService,JxDosResizeService,void *);
void jx_stream_allocate_buffer(JxEventContext *,uint16_t);
uint16_t jx_stream_overflow(JxEventContext *,uint16_t,JxFarPointer);
uint16_t jx_sprintf_format(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer,JxFarPointer);
uint16_t jx_sprintf_words(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer);
typedef void (*JxDosReadService)(void *,uint16_t,JxFarPointer,uint16_t,JxDosWriteResult *);
typedef void (*JxDosDeviceInfoService)(void *,uint16_t,JxDosSeekResult *);
/* One private byte is used for CR lookahead; services preserve BX/CX and
 * segments, read preserves DX, ioctl supplies DX. No host filesystem access. */
uint16_t jx_file_read(JxEventContext *,uint16_t,JxFarPointer,uint16_t,JxFarPointer,
    JxDosReadService,JxDosSeekService,JxDosDeviceInfoService,JxWriteHook,void *);
typedef struct {
    void *opaque;
    JxDosAllocateService allocate;
    JxDosResizeService resize;
    JxDosReadService read;
    JxDosSeekService seek;
    JxDosDeviceInfoService info;
    JxWriteHook hook;
    JxFarPointer lookahead;
} JxReadServices;
uint16_t jx_stream_refill(JxEventContext *,uint16_t);
uint16_t jx_stream_refill_dos(JxEventContext *,uint16_t,const JxReadServices *);
uint16_t jx_fread_dos(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t,const JxReadServices *);
void jx_free(JxEventContext *,JxFarPointer);
uint32_t jx_itoa16(JxEventContext *,uint16_t,JxFarPointer,uint16_t);
uint16_t jx_stream_flush(JxEventContext *,uint16_t);
void jx_stream_free_buffer(JxEventContext *,uint16_t);
/* Private 12-byte temporary filename workspace; original templates must fit. */
uint16_t jx_fclose(JxEventContext *,uint16_t,JxFarPointer);
/* DOS close/delete return AX/carry and preserve BX and segment registers. */
typedef void (*JxDosCloseService)(void *,uint16_t,JxDosWriteResult *);
typedef void (*JxDosDeleteService)(void *,JxFarPointer,JxDosWriteResult *);
uint16_t jx_file_close(JxEventContext *,uint16_t,JxDosCloseService,void *);
uint16_t jx_file_delete(JxEventContext *,JxFarPointer,JxDosDeleteService,void *);
typedef struct {
    const JxCrtServices *output;
    void *opaque;
    JxDosCloseService close;
    JxDosDeleteService remove;
} JxCloseServices;
JxFileWriteResult jx_stream_flush_dos(JxEventContext *,uint16_t,const JxCrtServices *);
JxFileWriteResult jx_fclose_dos(JxEventContext *,uint16_t,JxFarPointer,const JxCloseServices *);
typedef void (*JxDosOpenService)(void *,JxFarPointer,uint8_t,JxDosWriteResult *);
typedef void (*JxDosCreateService)(void *,JxFarPointer,uint16_t,JxDosWriteResult *);
typedef struct {uint16_t ax,attributes;uint8_t carry;} JxDosAttributeResult;
typedef void (*JxDosAttributeService)(void *,JxFarPointer,uint8_t,uint16_t,JxDosAttributeResult *);
/* File services preserve BX/CX/segments, except attributes explicitly returns
 * CX. Truncate means DOS write count=0; its unused buffer is not exposed.
 * tail_byte is one private, deliberately uninitialized byte, matching DOS CRT. */
typedef struct {
    void *opaque;
    JxDosOpenService open;
    JxDosCreateService create;
    JxDosAttributeService attributes;
    JxDosCloseService close,truncate;
    JxDosDeviceInfoService info;
    JxDosReadService read;
    JxDosSeekService seek;
    JxFarPointer tail_byte;
} JxOpenServices;
uint16_t jx_file_open(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t,const JxOpenServices *);
uint32_t jx_stream_open(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,uint16_t);
uint32_t jx_fopen_shared(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
uint32_t jx_fopen(JxEventContext *,JxFarPointer,JxFarPointer);
uint32_t jx_fopen_dos(JxEventContext *,JxFarPointer,JxFarPointer,const JxOpenServices *);
uint32_t jx_fopen_shared_dos(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,const JxOpenServices *);
uint32_t jx_ftell(JxEventContext *,uint16_t);
uint32_t jx_ftell_dos(JxEventContext *,uint16_t,JxDosSeekService,void *);
uint16_t jx_fseek(JxEventContext *,uint16_t,uint32_t,uint16_t);
JxFileWriteResult jx_fseek_dos(JxEventContext *,uint16_t,uint32_t,uint16_t,const JxCrtServices *);
uint32_t jx_stream_find_free(JxEventContext *);
/* fread chunk requires positive request and nonzero configured buffer size;
 * low-level read/getc remain explicit callbacks. */
uint16_t jx_stream_read_chunk(JxEventContext *,uint16_t,JxFarPointer,uint16_t);
uint16_t jx_fread(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t);
uint16_t jx_string_length(JxEventContext *,JxFarPointer);
int jx_font_load_double(JxEventContext *,uint16_t);
int jx_font_load_single(JxEventContext *,int16_t);
/* STALLED represents an original infinite loop after rasterizing row 0;
 * the host must stop/suspend this call, never continue as a normal return. */
typedef enum {JX_GLYPH_RETURNED=0,JX_GLYPH_STALLED=1} JxGlyphFlow;
JxGlyphFlow jx_font_raster_single(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
JxGlyphFlow jx_font_raster_double(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
typedef struct {JxGlyphFlow flow;int result;} JxTextResult;
JxTextResult jx_font_draw_text(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
void jx_portrait_draw(JxEventContext *,uint16_t,uint16_t,int16_t);
typedef struct {
    const JxOpenServices *open;
    const JxReadServices *read;
    const JxCloseServices *close;
    JxFarPointer close_workspace;
} JxStdioServices;
JxWriteFlow jx_portrait_draw_dos(JxEventContext *,uint16_t,uint16_t,int16_t,const JxStdioServices *);
void jx_font_set_colors(JxEventContext *,uint16_t,uint16_t,uint16_t);
void jx_font_set_mode(JxEventContext *,uint16_t);
int jx_palette_nearest_exact(JxEventContext *,uint8_t,uint8_t,uint8_t,JxFarPointer);
int jx_palette_build_map(JxEventContext *,JxFarPointer,uint32_t,JxFarPointer,float);
void jx_color_map_pixels(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,JxFarPointer);
uint16_t jx_character_is_digit(JxEventContext *,int16_t);
int jx_dialogue_fragment(JxEventContext *,int16_t,JxFarPointer,int16_t,JxFarPointer);
void jx_dialogue_run(JxEventContext *,int16_t,JxFarPointer,uint16_t,JxFarPointer);
void jx_transition_rows(JxEventContext *,JxFarPointer,uint16_t);
void jx_transition_columns(JxEventContext *,JxFarPointer,uint16_t);
void jx_transition_inward(JxEventContext *,JxFarPointer);
void jx_transition_outward(JxEventContext *,JxFarPointer);
void jx_transition_border_passes(JxEventContext *,JxFarPointer);
/* Device transactions, not a linear framebuffer. The host implements VGA
 * plane selection and aperture writes, or records them for verification. */
typedef struct {
    void *opaque;
    void (*out)(void *,uint16_t port,uint16_t value,unsigned width);
    void (*write_byte)(void *,uint32_t physical_address,uint8_t value);
} JxVideoDevice;
/* Separate read interface preserves the established write-device ABI. */
typedef struct {
    void *opaque;
    void (*out)(void *,uint16_t,uint16_t,unsigned);
    uint8_t (*read_byte)(void *,uint32_t);
} JxVideoReadDevice;
/* Combined bus for routines using IN and latch-preserving read/write pairs.
 * Existing read/write device layouts remain unchanged. */
typedef struct {
    void *opaque;
    void (*out)(void *,uint16_t,uint16_t,unsigned);
    uint8_t (*in_byte)(void *,uint16_t);
    uint8_t (*read_byte)(void *,uint32_t);
    void (*write_byte)(void *,uint32_t,uint8_t);
} JxVideoBus;
void jx_video_capture_planes(JxEventContext *,const JxVideoReadDevice *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer);
void jx_video_blit_planes(JxEventContext *,const JxVideoDevice *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer);
void jx_video_enter_mode_x(const JxVideoBus *,void (*bios_mode)(void *,uint16_t),void *bios_opaque);
void jx_video_blank_screen(const JxVideoBus *);
void jx_video_unblank_screen(const JxVideoBus *);
void jx_video_copy_rectangle(const JxVideoBus *,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
void jx_video_copy_page(const JxVideoBus *,uint16_t destination,uint16_t source);
/* Native local-palette transfers replace compiler-owned stack pointers. */
typedef struct {
    void *opaque;
    void (*read)(void *,uint16_t,uint16_t,uint8_t *);
    void (*write)(void *,uint16_t,uint16_t,const uint8_t *);
} JxPaletteIO;
/* Adapter lifetime spans the fade. Native RGB buffers contain count*3
 * bytes (1..256 colors); bus callbacks must not retain their pointers. */
typedef struct {
    const JxVideoBus *bus;
    void (*read)(void *,uint16_t,uint16_t,uint8_t *);
    void *read_opaque;
} JxPaletteBusAdapter;
JxPaletteIO jx_palette_bus_io(JxPaletteBusAdapter *);
typedef enum {JX_PALETTE_RETURNED=0,JX_PALETTE_DIVIDE_ERROR=1} JxPaletteFlow;
JxPaletteFlow jx_palette_fade_black(JxEventContext *,JxFarPointer,int16_t,const JxPaletteIO *);
JxPaletteFlow jx_palette_fade_to(JxEventContext *,JxFarPointer,int16_t,const JxPaletteIO *);
void jx_palette_write(JxEventContext *,const JxVideoBus *,JxFarPointer,uint16_t,uint16_t);
/* BIOS read service receives the exact original far destination and count. */
typedef void (*JxPaletteBiosRead)(void *,JxFarPointer,uint16_t,uint16_t);
void jx_palette_read(JxFarPointer,uint16_t,uint16_t,JxPaletteBiosRead,void *);
typedef void (*JxPaletteBiosWrite)(void *,JxFarPointer,uint16_t,uint16_t);
void jx_palette_bios_write(JxFarPointer,uint16_t,uint16_t,JxPaletteBiosWrite,void *);
void jx_palette_bios_bus_write(JxEventContext *,const JxVideoBus *,JxFarPointer,uint16_t,uint16_t);
void jx_palette_bios_grayscale(uint8_t *);
void jx_video_capture(JxEventContext *,const JxVideoReadDevice *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer);
typedef struct {uint16_t offset;uint8_t plane_mask,plane;} JxVideoAddress;
JxVideoAddress jx_video_address(uint16_t x,uint16_t y);
void jx_video_plot(JxEventContext *,const JxVideoDevice *,uint16_t,uint16_t,uint16_t);
/* DF=0; no clipping. Width zero skips all planes, height zero is 65536 rows. */
void jx_video_fill_rectangle(JxEventContext *,const JxVideoDevice *,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
void jx_video_draw_page(JxEventContext *,uint16_t);
void jx_video_show_page(const JxVideoDevice *,uint16_t);
void jx_video_blit(JxEventContext *,const JxVideoDevice *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer);
void jx_video_blit_transparent(JxEventContext *,const JxVideoDevice *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer,uint16_t);
void jx_video_blit_silhouette(JxEventContext *,const JxVideoDevice *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer,uint16_t,uint16_t);
/* Native event values replace pointers to compiler-owned DOS stack locals.
 * A poll adapter must supply the original dequeue contract: zero on success,
 * nonzero if empty. It also owns host scheduling/IRQ synchronization. */
typedef struct {uint16_t type, value1, value2, value3;} JxInputEvent;
typedef int (*JxInputPoll)(void *opaque,JxInputEvent *event);
/* Private 36-byte workspace preserves original text/number overlap layout;
 * valid item names and format must fit the original stack buffer. */
JxGlyphFlow jx_inventory_grant(JxEventContext *,int16_t,int16_t,JxFarPointer,JxInputPoll,void *);
/* Integrated formatting adds a separate private 534-byte workspace: parser
 * scratch plus one 16-bit argument. Neither workspace may wrap offset FFFF. */
JxGlyphFlow jx_inventory_grant_formatted(JxEventContext *,int16_t,int16_t,JxFarPointer,JxFarPointer,JxInputPoll,void *);
/* Native read-only NUL-terminated text replaces renderer-owned DOS locals. */
typedef void (*JxTextDraw)(void *opaque,uint16_t x,uint16_t y,const uint8_t *text);
void jx_dialogue_render(JxEventContext *,uint16_t,JxFarPointer,int16_t,
    JxInputPoll,JxTextDraw,void *);
void jx_dialogue_show(JxEventContext *,int16_t,JxFarPointer,uint16_t,
    JxFarPointer,JxInputPoll,JxTextDraw,void *);
JxGlyphFlow jx_dialogue_render_font(JxEventContext *,uint16_t,JxFarPointer,int16_t,
    JxInputPoll,void *,JxFarPointer);
JxGlyphFlow jx_dialogue_show_font(JxEventContext *,int16_t,JxFarPointer,uint16_t,
    JxFarPointer,JxInputPoll,void *,JxFarPointer);
/* Distinguish the original glyph loop from abnormal CRT stack control flow. */
typedef enum {JX_DIALOGUE_RETURNED=0,JX_DIALOGUE_GLYPH_STALLED=1,JX_DIALOGUE_STACK_FAILURE=2} JxDialogueFlow;
JxDialogueFlow jx_dialogue_render_stdio(JxEventContext *,uint16_t,JxFarPointer,int16_t,JxInputPoll,void *,JxFarPointer,const JxStdioServices *);
JxDialogueFlow jx_dialogue_show_stdio(JxEventContext *,int16_t,JxFarPointer,uint16_t,JxFarPointer,JxInputPoll,void *,JxFarPointer,const JxStdioServices *);
int jx_input_poll_queue(void *context,JxInputEvent *event);
void jx_delay(JxEventContext *,uint16_t,JxInputPoll,void *);
void jx_wait_key(JxEventContext *,JxInputPoll,void *);
void jx_wait_key_music(JxEventContext *,JxInputPoll,void *);
void jx_wait_key_timed(JxEventContext *,uint16_t,JxInputPoll,void *);
/* A fatal original exit is propagated to the host, which must terminate
 * game execution. Callers must not treat EXIT_1 as a normal event return. */
typedef enum {JX_FLOW_RETURNED=0,JX_FLOW_EXIT_1=1} JxFlow;
JxFlow jx_cinematic_180(JxEventContext *,JxFarPointer,JxFarPointer,JxInputPoll,void *);
JxFlow jx_cinematic_event_180(JxEventContext *,int16_t,JxInputPoll,void *);
void jx_board_draw(JxEventContext *);
void jx_board_play_commands(JxEventContext *,JxFarPointer);
int jx_board_load(JxEventContext *,int16_t);
typedef uint16_t (*JxTicksLow)(void *opaque);
int jx_board_run(JxEventContext *,int16_t,JxInputPoll,JxTicksLow,void *);
void jx_transition_wave(JxEventContext *,JxFarPointer,JxFarPointer,
    uint16_t,uint16_t,JxInputPoll,void *);
static inline uint8_t jx_far_byte(JxEventContext *c,JxFarPointer p,uint16_t displacement) {
    return c->memory[((uint32_t)p.segment*16+(uint16_t)(p.offset+displacement))&0xfffff];
}
static inline void jx_far_set_byte(JxEventContext *c,JxFarPointer p,uint16_t displacement,uint8_t value) {
    c->memory[((uint32_t)p.segment*16+(uint16_t)(p.offset+displacement))&0xfffff]=value;
}

static inline uint8_t *jx_ds(JxEventContext *c) {
    return c->memory + ((uint32_t)c->data_segment << 4);
}
static inline uint16_t jx_word(JxEventContext *c, uint16_t a) {
    uint8_t *d=jx_ds(c);
    return (uint16_t)(d[a] | ((uint16_t)d[(uint16_t)(a+1)] << 8));
}
static inline void jx_set_word(JxEventContext *c,uint16_t a,uint16_t v) {
    uint8_t *d=jx_ds(c); d[a]=(uint8_t)v; d[(uint16_t)(a+1)]=(uint8_t)(v>>8);
}
static inline int jx_signed_word(uint16_t v) { return v<32768 ? v : (int)v-65536; }
static inline int jx_signed_byte(uint8_t v) { return v<128 ? v : (int)v-256; }
static inline uint16_t jx_far_word(JxEventContext *c,uint16_t seg,uint16_t off) {
    uint32_t a=((uint32_t)seg*16+off)&0xfffff;
    return (uint16_t)(c->memory[a] | ((uint16_t)c->memory[(a+1)&0xfffff]<<8));
}
#define JX_CALL(c,s,o,...) ((c)->call((c)->opaque,(s),(o), \
    (const uint16_t[]){__VA_ARGS__},sizeof((const uint16_t[]){__VA_ARGS__})/sizeof(uint16_t)))
#define JX_CALL0(c,s,o) ((c)->call((c)->opaque,(s),(o),NULL,0))

/* Display/text helper 2F28:648E; the exact engine contract is still being named. */
static inline void jx_event_text(JxEventContext *c,int16_t npc,uint16_t table,int mode) {
    (void)JX_CALL(c,0x2f28,0x648e,(uint16_t)npc,jx_word(c,table),jx_word(c,(uint16_t)(table+2)),(uint16_t)mode);
}
static inline void jx_event_literal(JxEventContext *c,int16_t npc,uint16_t text,int mode) {
    (void)JX_CALL(c,0x2f28,0x648e,(uint16_t)npc,text,c->data_segment,(uint16_t)mode);
}
static inline void jx_event_refresh(JxEventContext *c) { (void)JX_CALL0(c,0x2f28,0x5580); }
static inline void jx_event_transition_begin(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
}
static inline void jx_event_transition_end(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
}
static inline void jx_event_grant(JxEventContext *c,uint16_t item,uint16_t count) {
    (void)JX_CALL(c,0x1f2a,0x8cea,item,count);
}
static inline void jx_event_position(JxEventContext *c,uint16_t base,uint16_t x,uint16_t y,uint16_t facing) {
    jx_set_word(c,base,x);jx_set_word(c,(uint16_t)(base+2),y);jx_set_word(c,(uint16_t)(base+4),facing);
}
static inline void jx_event_music(JxEventContext *c,uint16_t track,uint16_t mode) {
    jx_set_word(c,0x889a,track);
    if(jx_signed_word(jx_word(c,0x31b4))>0 && jx_signed_word(jx_word(c,0x050c))>0) {
        uint16_t resource=jx_far_word(c,jx_word(c,0x23ae),(uint16_t)(0x510+2*track));
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track)&255),mode,resource);
    }
}
static inline void jx_event_state_text(JxEventContext *c,int16_t npc,uint16_t state_base) {
    int state=jx_signed_byte(jx_ds(c)[(uint16_t)(state_base+npc)]);
    uint16_t slot=(uint16_t)(0x889c+(uint16_t)npc*86u+0x2c+state*2);
    jx_event_text(c,npc,(uint16_t)(0x59d6+4u*jx_word(c,slot)),1);
}

void jx_event_015_0000(JxEventContext *,int16_t);
void jx_event_005_0000(JxEventContext *,int16_t);
void jx_event_005_0594(JxEventContext *,int16_t);
void jx_event_018_0000(JxEventContext *,int16_t);
void jx_event_018_00aa(JxEventContext *,int16_t);
void jx_event_022_0000(JxEventContext *,int16_t);
void jx_event_031_0000(JxEventContext *,int16_t);
void jx_event_031_002c(JxEventContext *,int16_t);
void jx_event_033_0000(JxEventContext *,int16_t);
void jx_event_043_0000(JxEventContext *,int16_t);
void jx_event_058_0000(JxEventContext *,int16_t);
void jx_event_111_0000(JxEventContext *,int16_t);
void jx_event_123_0000(JxEventContext *,int16_t);
void jx_event_156_0000(JxEventContext *,int16_t);
void jx_event_006_0000(JxEventContext *,int16_t);
void jx_event_006_0132(JxEventContext *,int16_t);
void jx_event_011_0000(JxEventContext *,int16_t);
void jx_event_011_001e(JxEventContext *,int16_t);
void jx_event_011_004c(JxEventContext *,int16_t);
void jx_event_011_00c8(JxEventContext *,int16_t);
void jx_event_011_014e(JxEventContext *,int16_t);
void jx_event_014_0000(JxEventContext *,int16_t);
void jx_event_014_0064(JxEventContext *,int16_t);
void jx_event_017_0000(JxEventContext *,int16_t);
void jx_event_017_008c(JxEventContext *,int16_t);
void jx_event_017_00f4(JxEventContext *,int16_t);
void jx_camera_lock(JxEventContext *,int16_t);
void jx_camera_unlock(JxEventContext *,int16_t);
void jx_scene_redraw(JxEventContext *,int16_t);
void jx_npc_play_animation(JxEventContext *,int16_t);
void jx_npc_reverse_animation(JxEventContext *,int16_t);
void jx_npc_redraw_background(JxEventContext *,int16_t);
void jx_npc_run_moves(JxEventContext *,uint16_t,JxFarPointer,uint16_t);
uint8_t jx_choose_step(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t);
typedef enum {JX_CELL_RETURNED=0,JX_CELL_DIVIDE_ERROR=1} JxCellFlow;
JxCellFlow jx_scene_cell_layers(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_map_cell_info(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,uint16_t,uint16_t);
uint16_t jx_actor_cell_info(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
uint16_t jx_actor_alternate_cell_info(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
uint16_t jx_movement_classify(JxEventContext *,uint16_t,uint16_t);
uint32_t jx_string_uppercase(JxEventContext *,JxFarPointer);
int16_t jx_string_compare(JxEventContext *,JxFarPointer,JxFarPointer);
uint32_t jx_archive_open(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer);
typedef enum {JX_MAP_LOAD_RETURNED=0,JX_MAP_LOAD_FATAL=1} JxMapLoadFlow;
JxMapLoadFlow jx_map_load(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_place_load(JxEventContext *,uint16_t);
uint16_t jx_event_clear_state(JxEventContext *);
void jx_event_empty(JxEventContext *);
void jx_party_check_triggers(JxEventContext *);
JxWorldUpdateFlow jx_party_check_triggers_world(JxEventContext *,JxFarPointer);
void jx_party_advance_frames(JxEventContext *);
void jx_party_shift_trail(JxEventContext *);
void jx_party_redraw_trail(JxEventContext *);
JxCellFlow jx_party_redraw_trail_cells(JxEventContext *,JxFarPointer);
JxCellFlow jx_party_redraw_page_cells(JxEventContext *,JxFarPointer);
JxCellFlow jx_party_scroll_page_cells(JxEventContext *,uint16_t,JxFarPointer);
void jx_party_redraw_page(JxEventContext *);
void jx_party_scroll_page(JxEventContext *,uint16_t);
void jx_party_move_cycle(JxEventContext *,uint16_t,uint16_t);
JxCellFlow jx_party_move_cycle_cells(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
JxCellFlow jx_party_run_moves_cells(JxEventContext *,JxFarPointer,uint16_t,JxFarPointer);
void jx_party_run_moves(JxEventContext *,JxFarPointer,uint16_t);
void jx_party_walk_to(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_tile_map_resource(JxEventContext *,uint16_t,uint16_t);
void jx_scene_draw_tile(JxEventContext *,int16_t,int16_t);
JxCellFlow jx_scene_redraw_cells(JxEventContext *,JxFarPointer);
JxCellFlow jx_scene_draw_tile_cells(JxEventContext *,int16_t,int16_t,JxFarPointer);
void jx_camera_scroll(JxEventContext *,uint16_t,int16_t);
int jx_rectangle_local_point(uint16_t *,uint16_t *,uint16_t,uint16_t,uint16_t,uint16_t,int16_t,int16_t);

typedef struct {
    JxEventContext *context;
    JxEngineCall fallback;
    void *fallback_opaque;
} JxEngineBridge;
typedef struct {JxEngineBridge base;JxWorldUpdateFlow flow;
    uint16_t fault_segment,fault_offset;} JxWorldStateBridge;
void jx_world_state_bridge_install(JxEventContext *,JxWorldStateBridge *);
void jx_world_state_bridge_remove(JxEventContext *,JxWorldStateBridge *);
int jx_world_state_bridge_can_step(const JxWorldStateBridge *);
/* Four guest scratch bytes, disjoint from live memory; preserve contents
 * across calls because failed EMS queries leave original locals unchanged. */
typedef struct {JxWorldStateBridge state;JxFarPointer scratch;} JxWorldCollisionBridge;
void jx_world_collision_bridge_install(JxEventContext *,JxWorldCollisionBridge *,JxFarPointer);
void jx_world_collision_bridge_remove(JxEventContext *,JxWorldCollisionBridge *);
void jx_world_update_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_world_update_bridge_remove(JxEventContext *,JxEngineBridge *);
typedef struct {JxEngineBridge base;uint16_t active;} JxGameControlBridge;
void jx_game_control_bridge_install(JxEventContext *,JxGameControlBridge *);
void jx_game_control_bridge_remove(JxEventContext *,JxGameControlBridge *);
typedef struct {
    JxEngineBridge base;
    JxVideoBus bus;
    void (*bios_mode)(void *,uint16_t);
    void *bios_opaque;
} JxVideoStartupBridge;
void jx_video_startup_bridge_install(JxEventContext *,JxVideoStartupBridge *,const JxVideoBus *,void (*)(void *,uint16_t),void *);
void jx_video_startup_bridge_remove(JxEventContext *,JxVideoStartupBridge *);
typedef struct {JxEngineBridge base;JxEmsDevice device;} JxEmsBridge;
typedef struct {JxEngineBridge base;JxCdDevice device;JxFarPointer table,scratch;} JxCdBridge;
void jx_cd_bridge_install(JxEventContext *,JxCdBridge *,const JxCdDevice *,JxFarPointer,JxFarPointer);
void jx_cd_bridge_remove(JxEventContext *,JxCdBridge *);
void jx_ems_bridge_install(JxEventContext *,JxEmsBridge *,const JxEmsDevice *);
void jx_ems_bridge_remove(JxEventContext *,JxEmsBridge *);
typedef struct {
    JxEngineBridge base;
    JxVideoBus bus;
} JxVideoBridge;
void jx_video_bridge_install(JxEventContext *,JxVideoBridge *,const JxVideoBus *);
void jx_video_bridge_remove(JxEventContext *,JxVideoBridge *);
/* Connect recovered bodies; remaining lower-level calls go to the host.
 * bridge must outlive context use; do not install it twice on one context. */
void jx_engine_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_engine_bridge_remove(JxEventContext *,JxEngineBridge *);
/* Optional seven-function CRT layer (memory, strings, random, multiply).
 * Layers use distinct bridge objects and must be removed in reverse order.
 * Word returns use AX; srand's untouched caller DX is not represented.
 * Unrecognized addresses forward unchanged to the saved host callback. */
void jx_runtime_bridge_install(JxEventContext *,JxEngineBridge *);
/* Optional five-function palette/pixel-memory layer; same lifetime and
 * reverse-order removal rules as the CRT and scene bridges. */
void jx_inventory_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_graphics_memory_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_event_035_0000(JxEventContext *,int16_t);
void jx_event_035_00e8(JxEventContext *,int16_t);
void jx_event_046_0000(JxEventContext *,int16_t);
void jx_event_046_00be(JxEventContext *,int16_t);
void jx_event_086_0000(JxEventContext *,int16_t);
void jx_event_086_0130(JxEventContext *,int16_t);
void jx_event_023_0000(JxEventContext *,int16_t);
void jx_event_023_00ba(JxEventContext *,int16_t);
void jx_event_024_0000(JxEventContext *,int16_t);
void jx_event_026_0000(JxEventContext *,int16_t);
void jx_event_026_009c(JxEventContext *,int16_t);
void jx_event_090_0000(JxEventContext *,int16_t);
void jx_event_103_0000(JxEventContext *,int16_t);
void jx_event_103_00fa(JxEventContext *,int16_t);
void jx_event_103_015a(JxEventContext *,int16_t);
void jx_event_131_0000(JxEventContext *,int16_t);
void jx_event_131_011c(JxEventContext *,int16_t);
void jx_event_037_0000(JxEventContext *,int16_t);
void jx_event_037_0064(JxEventContext *,int16_t);
void jx_event_083_0000(JxEventContext *,int16_t);
void jx_event_101_0000(JxEventContext *,int16_t);
void jx_event_101_009e(JxEventContext *,int16_t);
void jx_event_152_0000(JxEventContext *,int16_t);
void jx_event_159_0000(JxEventContext *,int16_t);
void jx_event_163_0000(JxEventContext *,int16_t);
void jx_event_010_0000(JxEventContext *,int16_t);
void jx_event_010_0066(JxEventContext *,int16_t);
void jx_event_010_00c0(JxEventContext *,int16_t);
void jx_event_010_00ee(JxEventContext *,int16_t);
void jx_event_010_01ac(JxEventContext *,int16_t);
void jx_event_010_0226(JxEventContext *,int16_t);
void jx_event_010_028c(JxEventContext *,int16_t);
void jx_event_025_0000(JxEventContext *,int16_t);
void jx_event_025_005c(JxEventContext *,int16_t);
void jx_event_025_0126(JxEventContext *,int16_t);
void jx_event_025_0162(JxEventContext *,int16_t);
void jx_event_027_0000(JxEventContext *,int16_t);
void jx_event_027_0072(JxEventContext *,int16_t);
void jx_event_027_0124(JxEventContext *,int16_t);
void jx_event_027_01e4(JxEventContext *,int16_t);
void jx_event_027_02ca(JxEventContext *,int16_t);
void jx_event_019_0000(JxEventContext *,int16_t);
void jx_event_019_0376(JxEventContext *,int16_t);
void jx_event_178_0000(JxEventContext *,int16_t);
void jx_event_072_0000(JxEventContext *,int16_t);
void jx_event_181_0000(JxEventContext *,int16_t);
void jx_event_057_0000(JxEventContext *,int16_t);
void jx_event_130_0000(JxEventContext *,int16_t);
void jx_event_197_0000(JxEventContext *,int16_t);
int jx_input_dequeue(JxEventContext *,uint16_t,uint16_t);
int jx_input_enqueue(JxEventContext *,JxFarPointer);
uint8_t jx_keyboard_translate(JxEventContext *,uint16_t);
uint16_t jx_keyboard_set_direction(JxEventContext *,uint16_t,uint16_t);
void jx_input_clear(JxEventContext *);
void jx_music_poll(JxEventContext *);
void jx_music_toggle(JxEventContext *);
void jx_sound_toggle(JxEventContext *);
void jx_music_restart(JxEventContext *);
void jx_party_interact(JxEventContext *);
void jx_party_present_item(JxEventContext *,uint16_t);
void jx_party_actor_menus(JxEventContext *);
void jx_party_skill_menus(JxEventContext *);
void jx_party_use_item(JxEventContext *);
void jx_event_194_0000(JxEventContext *,int16_t);
void jx_event_194_088c(JxEventContext *,int16_t);
void jx_transition_interlaced(JxEventContext *,uint16_t,uint16_t);
void jx_transition_cascade(JxEventContext *,uint16_t,uint16_t);
int jx_palette_nearest(JxEventContext *,uint8_t,uint8_t,uint8_t,JxFarPointer,int16_t);
void jx_transition_blend(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t,
    JxFarPointer,JxFarPointer,JxFarPointer,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer path;} JxMovementBridge;
void jx_movement_bridge_install(JxEventContext *,JxMovementBridge *,JxFarPointer);
void jx_movement_bridge_remove(JxEventContext *,JxMovementBridge *);
void jx_trigger_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_trigger_bridge_remove(JxEventContext *,JxEngineBridge *);

void jx_npc_reset(JxEventContext *);
void jx_scene_objects_reset(JxEventContext *);
void jx_scene_sprites_load(JxEventContext *);
void jx_sprite_load(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_animation_cache(JxEventContext *,JxFarPointer,uint16_t);
void jx_weather_load(JxEventContext *);
void jx_sound_resource_load(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer sprite,archive,sound;} JxResourceBridge;
void jx_resource_bridge_install(JxEventContext *,JxResourceBridge *,JxFarPointer,JxFarPointer,JxFarPointer);
void jx_resource_bridge_remove(JxEventContext *,JxResourceBridge *);
typedef struct {uint16_t source_offset,destination_offset;uint8_t carry;} JxAnimationPacketResult;
JxAnimationPacketResult jx_animation_packet(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
typedef void (*JxDosFreeService)(void *,uint16_t segment);
typedef struct {
    void *opaque;
    JxDosOpenService open;
    JxDosAllocateService allocate;
    JxDosReadService read;
    JxDosSeekService seek;
    JxDosCloseService close;
    JxDosFreeService free_block;
} JxAnimationServices;
uint16_t jx_animation_open(JxEventContext *,JxFarPointer,JxFarPointer,const JxAnimationServices *);
void jx_animation_close(JxEventContext *,JxFarPointer,const JxAnimationServices *);
void jx_animation_first(JxEventContext *,JxFarPointer,JxFarPointer,const JxAnimationServices *);
void jx_animation_next(JxEventContext *,JxFarPointer,JxFarPointer,const JxAnimationServices *);
typedef struct {JxEngineBridge base;const JxAnimationServices *services;} JxAnimationBridge;
void jx_animation_bridge_install(JxEventContext *,JxAnimationBridge *,const JxAnimationServices *);
void jx_animation_bridge_remove(JxEventContext *,JxAnimationBridge *);
uint16_t jx_getw(JxEventContext *,uint16_t);
uint16_t jx_getw_dos(JxEventContext *,uint16_t,const JxReadServices *);
uint16_t jx_scene_objects_load(JxEventContext *,uint16_t);
void jx_scene_backdrop_load(JxEventContext *,JxFarPointer);
typedef enum {JX_RESOURCE_INIT_RETURNED=0,JX_RESOURCE_INIT_EXIT=1} JxResourceInitFlow;
JxResourceInitFlow jx_resources_initialize(JxEventContext *,JxFarPointer);
void jx_font_resources_load(JxEventContext *);
/* Proven static and empty place-entry functions. */
void jx_place_enter_002(JxEventContext *);
void jx_place_enter_003(JxEventContext *);
void jx_place_enter_004(JxEventContext *);
void jx_place_enter_006(JxEventContext *);
void jx_place_enter_007(JxEventContext *);
void jx_place_enter_008(JxEventContext *);
void jx_place_enter_009(JxEventContext *);
void jx_place_enter_010(JxEventContext *);
void jx_place_enter_011(JxEventContext *);
void jx_place_enter_012(JxEventContext *);
void jx_place_enter_013(JxEventContext *);
void jx_place_enter_014(JxEventContext *);
void jx_place_enter_015(JxEventContext *);
void jx_place_enter_016(JxEventContext *);
void jx_place_enter_017(JxEventContext *);
void jx_place_enter_018(JxEventContext *);
void jx_place_enter_019(JxEventContext *);
void jx_place_enter_021(JxEventContext *);
void jx_place_enter_023(JxEventContext *);
void jx_place_enter_024(JxEventContext *);
void jx_place_enter_025(JxEventContext *);
void jx_place_enter_026(JxEventContext *);
void jx_place_enter_027(JxEventContext *);
void jx_place_enter_028(JxEventContext *);
void jx_place_enter_029(JxEventContext *);
void jx_place_enter_030(JxEventContext *);
void jx_place_enter_031(JxEventContext *);
void jx_place_enter_033(JxEventContext *);
void jx_place_enter_034(JxEventContext *);
void jx_place_enter_035(JxEventContext *);
void jx_place_enter_036(JxEventContext *);
void jx_place_enter_037(JxEventContext *);
void jx_place_enter_038(JxEventContext *);
void jx_place_enter_039(JxEventContext *);
void jx_place_enter_040(JxEventContext *);
void jx_place_enter_041(JxEventContext *);
void jx_place_enter_042(JxEventContext *);
void jx_place_enter_044(JxEventContext *);
void jx_place_enter_045(JxEventContext *);
void jx_place_enter_046(JxEventContext *);
void jx_place_enter_047(JxEventContext *);
void jx_place_enter_048(JxEventContext *);
void jx_place_enter_049(JxEventContext *);
void jx_place_enter_050(JxEventContext *);
void jx_place_enter_051(JxEventContext *);
void jx_place_enter_052(JxEventContext *);
void jx_place_enter_053(JxEventContext *);
void jx_place_enter_054(JxEventContext *);
void jx_place_enter_055(JxEventContext *);
void jx_place_enter_056(JxEventContext *);
void jx_place_enter_057(JxEventContext *);
void jx_place_enter_058(JxEventContext *);
void jx_place_enter_059(JxEventContext *);
void jx_place_enter_060(JxEventContext *);
void jx_place_enter_061(JxEventContext *);
void jx_place_enter_062(JxEventContext *);
void jx_place_enter_063(JxEventContext *);
void jx_place_enter_064(JxEventContext *);
void jx_place_enter_065(JxEventContext *);
void jx_place_enter_066(JxEventContext *);
void jx_place_enter_068(JxEventContext *);
void jx_place_enter_069(JxEventContext *);
void jx_place_enter_070(JxEventContext *);
void jx_place_enter_071(JxEventContext *);
void jx_place_enter_072(JxEventContext *);
void jx_place_enter_073(JxEventContext *);
void jx_place_enter_074(JxEventContext *);
void jx_place_enter_075(JxEventContext *);
void jx_place_enter_076(JxEventContext *);
void jx_place_enter_077(JxEventContext *);
void jx_place_enter_078(JxEventContext *);
void jx_place_enter_079(JxEventContext *);
void jx_place_enter_080(JxEventContext *);
void jx_place_enter_082(JxEventContext *);
void jx_place_enter_083(JxEventContext *);
void jx_place_enter_084(JxEventContext *);
void jx_place_enter_085(JxEventContext *);
void jx_place_enter_086(JxEventContext *);
void jx_place_enter_087(JxEventContext *);
void jx_place_enter_088(JxEventContext *);
void jx_place_enter_089(JxEventContext *);
void jx_place_enter_090(JxEventContext *);
void jx_place_enter_091(JxEventContext *);
void jx_place_enter_093(JxEventContext *);
void jx_place_enter_094(JxEventContext *);
void jx_place_enter_095(JxEventContext *);
void jx_place_enter_096(JxEventContext *);
void jx_place_enter_097(JxEventContext *);
void jx_place_enter_098(JxEventContext *);
void jx_place_enter_099(JxEventContext *);
void jx_place_enter_100(JxEventContext *);
void jx_place_enter_101(JxEventContext *);
void jx_place_enter_102(JxEventContext *);
void jx_place_enter_103(JxEventContext *);
void jx_place_enter_104(JxEventContext *);
void jx_place_enter_105(JxEventContext *);
void jx_place_enter_106(JxEventContext *);
void jx_place_enter_107(JxEventContext *);
void jx_place_enter_108(JxEventContext *);
void jx_place_enter_109(JxEventContext *);
void jx_place_enter_110(JxEventContext *);
void jx_place_enter_111(JxEventContext *);
void jx_place_enter_112(JxEventContext *);
void jx_place_enter_113(JxEventContext *);
void jx_place_enter_114(JxEventContext *);
void jx_place_enter_115(JxEventContext *);
void jx_place_enter_116(JxEventContext *);
void jx_place_enter_117(JxEventContext *);
void jx_place_enter_119(JxEventContext *);
void jx_place_enter_120(JxEventContext *);
void jx_place_enter_122(JxEventContext *);
void jx_place_enter_123(JxEventContext *);
void jx_place_enter_124(JxEventContext *);
void jx_place_enter_125(JxEventContext *);
void jx_place_enter_127(JxEventContext *);
void jx_place_enter_128(JxEventContext *);
void jx_place_enter_129(JxEventContext *);
void jx_place_enter_130(JxEventContext *);
void jx_place_enter_131(JxEventContext *);
void jx_place_enter_132(JxEventContext *);
void jx_place_enter_133(JxEventContext *);
void jx_place_enter_134(JxEventContext *);
void jx_place_enter_135(JxEventContext *);
void jx_place_enter_136(JxEventContext *);
void jx_place_enter_137(JxEventContext *);
void jx_place_enter_138(JxEventContext *);
void jx_place_enter_139(JxEventContext *);
void jx_place_enter_140(JxEventContext *);
void jx_place_enter_141(JxEventContext *);
void jx_place_enter_142(JxEventContext *);
void jx_place_enter_143(JxEventContext *);
void jx_place_enter_144(JxEventContext *);
void jx_place_enter_145(JxEventContext *);
void jx_place_enter_146(JxEventContext *);
void jx_place_enter_148(JxEventContext *);
void jx_place_enter_149(JxEventContext *);
void jx_place_enter_150(JxEventContext *);
void jx_place_enter_151(JxEventContext *);
void jx_place_enter_152(JxEventContext *);
void jx_place_enter_153(JxEventContext *);
void jx_place_enter_155(JxEventContext *);
void jx_place_enter_156(JxEventContext *);
void jx_place_enter_157(JxEventContext *);
void jx_place_enter_158(JxEventContext *);
void jx_place_enter_159(JxEventContext *);
void jx_place_enter_160(JxEventContext *);
void jx_place_enter_162(JxEventContext *);
void jx_place_enter_163(JxEventContext *);
void jx_place_enter_164(JxEventContext *);
void jx_place_enter_165(JxEventContext *);
void jx_place_enter_166(JxEventContext *);
void jx_place_enter_167(JxEventContext *);
void jx_place_enter_168(JxEventContext *);
void jx_place_enter_169(JxEventContext *);
void jx_place_enter_170(JxEventContext *);
void jx_place_enter_174(JxEventContext *);
void jx_place_enter_175(JxEventContext *);
void jx_place_enter_176(JxEventContext *);
void jx_place_enter_177(JxEventContext *);
void jx_place_enter_178(JxEventContext *);
void jx_place_enter_179(JxEventContext *);
void jx_place_enter_180(JxEventContext *);
void jx_place_enter_181(JxEventContext *);
void jx_place_enter_182(JxEventContext *);
void jx_place_enter_183(JxEventContext *);
void jx_place_enter_184(JxEventContext *);
void jx_place_enter_186(JxEventContext *);
void jx_place_enter_187(JxEventContext *);
void jx_place_enter_188(JxEventContext *);
void jx_place_enter_189(JxEventContext *);
void jx_place_enter_190(JxEventContext *);
void jx_place_enter_191(JxEventContext *);
void jx_place_enter_192(JxEventContext *);
void jx_place_enter_193(JxEventContext *);
void jx_place_enter_194(JxEventContext *);
void jx_place_enter_195(JxEventContext *);
void jx_place_enter_196(JxEventContext *);
void jx_place_enter_197(JxEventContext *);
void jx_place_enter_198(JxEventContext *);
void jx_place_enter_199(JxEventContext *);
void jx_place_enter_200(JxEventContext *);
void jx_place_enter_201(JxEventContext *);
void jx_place_enter_202(JxEventContext *);
void jx_place_enter_203(JxEventContext *);
void jx_place_enter_204(JxEventContext *);
void jx_place_enter_205(JxEventContext *);
void jx_place_enter_206(JxEventContext *);
void jx_place_enter_207(JxEventContext *);
void jx_place_enter_208(JxEventContext *);
void jx_place_enter_209(JxEventContext *);
void jx_place_enter_210(JxEventContext *);
void jx_place_enter_211(JxEventContext *);
void jx_place_enter_212(JxEventContext *);
void jx_place_enter_214(JxEventContext *);
void jx_place_enter_215(JxEventContext *);
void jx_place_enter_216(JxEventContext *);
void jx_place_enter_217(JxEventContext *);
void jx_place_enter_218(JxEventContext *);
void jx_place_enter_219(JxEventContext *);
void jx_place_enter_001(JxEventContext *);
void jx_place_enter_005(JxEventContext *);
void jx_place_enter_020(JxEventContext *);
void jx_place_enter_043(JxEventContext *);
void jx_place_enter_121(JxEventContext *);
void jx_place_enter_171(JxEventContext *);
void jx_place_enter_172(JxEventContext *);
void jx_place_enter_000(JxEventContext *);
void jx_place_enter_067(JxEventContext *);
void jx_place_enter_081(JxEventContext *);
void jx_place_enter_118(JxEventContext *);
void jx_place_enter_126(JxEventContext *);
void jx_place_enter_173(JxEventContext *);
void jx_place_enter_092(JxEventContext *);
void jx_place_enter_147(JxEventContext *);
void jx_place_enter_032(JxEventContext *);
void jx_place_enter_154(JxEventContext *);
void jx_place_enter_185(JxEventContext *);
void jx_place_enter_161(JxEventContext *);
void jx_place_enter_213(JxEventContext *);
void jx_place_enter_022(JxEventContext *);
void jx_place_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_place_bridge_remove(JxEventContext *,JxEngineBridge *);
void jx_callback_1f2a_dd0a(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_ddea(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_dfc0(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_dfd6(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_dfec(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e002(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e018(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e166(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e17c(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e27c(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e292(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e2ca(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e404(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e43c(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e452(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e49e(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e4ca(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e4e0(JxEventContext *);
void jx_callback_1f2a_e588(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e59e(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e662(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e702(JxEventContext *);
void jx_callback_1f2a_e780(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e980(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e9c2(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e9d8(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_ea96(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_eb5c(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_ed94(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_eede(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_efd4(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_efea(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_f32e(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_fcfc(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_fe0c(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_ff8e(JxEventContext *,uint16_t npc);
void jx_callback_2f28_0204(JxEventContext *,uint16_t npc);
void jx_callback_2f28_021a(JxEventContext *,uint16_t npc);
void jx_callback_2f28_0ef8(JxEventContext *);
void jx_callback_2f28_0f5a(JxEventContext *,uint16_t npc);
void jx_callback_2f28_16e6(JxEventContext *,uint16_t npc);
void jx_callback_2f28_18d6(JxEventContext *,uint16_t npc);
void jx_callback_2f28_1b32(JxEventContext *,uint16_t npc);
void jx_callback_2f28_21c8(JxEventContext *,uint16_t npc);
void jx_callback_2f28_21f4(JxEventContext *,uint16_t npc);
void jx_callback_2f28_2544(JxEventContext *,uint16_t npc);
void jx_callback_2f28_34f4(JxEventContext *,uint16_t npc);
void jx_callback_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_callback_bridge_remove(JxEventContext *,JxEngineBridge *);
void jx_overlay_event_bridge_install(JxEventContext *,JxEngineBridge *);
void jx_overlay_event_bridge_remove(JxEventContext *,JxEngineBridge *);
void jx_callback_1f2a_ddcc(JxEventContext *);
void jx_callback_1f2a_dfa2(JxEventContext *);
void jx_callback_1f2a_e350(JxEventContext *);
void jx_callback_1f2a_e5d8(JxEventContext *);
void jx_callback_1f2a_e796(JxEventContext *);
void jx_callback_1f2a_eb3e(JxEventContext *);
void jx_callback_1f2a_edaa(JxEventContext *);
void jx_callback_1f2a_f2b4(JxEventContext *);
void jx_callback_1f2a_f422(JxEventContext *);
void jx_callback_2f28_011c(JxEventContext *);
void jx_callback_2f28_19d8(JxEventContext *);
void jx_callback_2f28_26fe(JxEventContext *);
void jx_callback_2f28_320e(JxEventContext *);
void jx_callback_2f28_2fc8(JxEventContext *c);
void jx_callback_1f2a_ec20(JxEventContext *c);
void jx_callback_2f28_1b0c(JxEventContext *c);
void jx_callback_1f2a_f7e2(JxEventContext *c);
void jx_callback_1f2a_e648(JxEventContext *c,uint16_t npc);
void jx_callback_1f2a_e76a(JxEventContext *c,uint16_t npc);
void jx_callback_1f2a_ee7e(JxEventContext *c);
void jx_callback_1f2a_e02e(JxEventContext *c);
void jx_callback_1f2a_e928(JxEventContext *c);
void jx_callback_1f2a_e966(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_1074(JxEventContext *c);
void jx_callback_1f2a_ebd0(JxEventContext *c);
void jx_callback_1f2a_f9f2(JxEventContext *c);
void jx_callback_2f28_18b6(JxEventContext *c);
void jx_callback_1f2a_f5f6(JxEventContext *);
void jx_callback_1f2a_e32c(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e54e(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e5b4(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e6de(JxEventContext *,uint16_t npc);
void jx_callback_2f28_02be(JxEventContext *,uint16_t npc);
void jx_callback_2f28_1ad0(JxEventContext *,uint16_t npc);
void jx_callback_2f28_2220(JxEventContext *);
void jx_callback_1f2a_e192(JxEventContext *);
void jx_callback_1f2a_e1bc(JxEventContext *);
void jx_callback_1f2a_e300(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e3d8(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_ebee(JxEventContext *);
void jx_callback_1f2a_ed6c(JxEventContext *);
void jx_callback_1f2a_ef76(JxEventContext *);
void jx_callback_1f2a_f440(JxEventContext *);
void jx_callback_1f2a_f59a(JxEventContext *);
void jx_callback_1f2a_ff0c(JxEventContext *,uint16_t npc);
void jx_callback_2f28_0290(JxEventContext *);
void jx_callback_2f28_060c(JxEventContext *,uint16_t npc);
void jx_callback_2f28_1cd0(JxEventContext *,uint16_t npc);
void jx_callback_2f28_34c6(JxEventContext *);
void jx_callback_1f2a_e712(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_e73e(JxEventContext *,uint16_t npc);
void jx_callback_1f2a_f364(JxEventContext *);
void jx_callback_1f2a_faa4(JxEventContext *,uint16_t npc);
void jx_callback_2f28_00ce(JxEventContext *,uint16_t npc);
void jx_callback_2f28_025c(JxEventContext *,uint16_t npc);
void jx_callback_2f28_039e(JxEventContext *);
void jx_callback_2f28_0724(JxEventContext *,uint16_t npc);
void jx_callback_2f28_0792(JxEventContext *,uint16_t npc);
void jx_callback_2f28_1cfa(JxEventContext *);
void jx_callback_2f28_1f52(JxEventContext *,uint16_t npc);
void jx_callback_2f28_2b2a(JxEventContext *,uint16_t npc);
void jx_callback_2f28_2e34(JxEventContext *,uint16_t npc);
void jx_callback_2f28_2ffc(JxEventContext *,uint16_t npc);
void jx_callback_2f28_3266(JxEventContext *);
void jx_scene_portrait_pair(JxEventContext *);
void jx_callback_2f28_2b58(JxEventContext *,uint16_t);
void jx_callback_2f28_2592(JxEventContext *,uint16_t);
void jx_scene_composite_frame(JxEventContext *,uint16_t,uint16_t,uint16_t);
void jx_lake_cutscene(JxEventContext *,JxFarPointer);
uint16_t jx_animation_crop_cache(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
void jx_image_present(JxEventContext *c,JxFarPointer filename);
void jx_image_background_present(JxEventContext *c,JxFarPointer filename);
void jx_image_tail_load(JxEventContext *c,JxFarPointer filename);
void jx_credits_cache(JxEventContext *);
void jx_credits_scroll_frame(JxEventContext *,uint16_t);
void jx_credits_run(JxEventContext *,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer event_work;} JxCutsceneBridge;
void jx_cutscene_bridge_install(JxEventContext *,JxCutsceneBridge *,JxFarPointer);
void jx_cutscene_bridge_remove(JxEventContext *,JxCutsceneBridge *);
void jx_actor_restore_current(JxEventContext *,uint16_t);
void jx_actor_level_up(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer text_work;} JxActorBridge;
/* text_work reserves 66 bytes outside game state and graphics buffers. */
void jx_actor_bridge_install(JxEventContext *,JxActorBridge *,JxFarPointer);
void jx_actor_bridge_remove(JxEventContext *,JxActorBridge *);
void jx_npc_walk_to(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_sound_play(JxEventContext *,uint16_t);
uint8_t jx_sound_dsp_reset(JxEventContext *,uint16_t);
void jx_sound_initialize(JxEventContext *);
uint16_t jx_sound_dsp_read(JxEventContext *,uint16_t);
uint8_t jx_sound_detect_version(JxEventContext *,JxFarPointer);
void jx_sound_install_irq(JxEventContext *,JxFarPointer,void (*)(void *,uint8_t),void *);
void jx_sound_irq(JxEventContext *);
void jx_crt_clear_bss(JxEventContext *);
void jx_world_random_encounter(JxEventContext *);
uint16_t jx_world_direction_tiles(JxEventContext *,JxFarPointer);
uint16_t jx_world_sinking_tiles(JxEventContext *,JxFarPointer);
uint16_t jx_world_lethal_tiles(JxEventContext *,JxFarPointer);
void jx_world_shift_page(JxEventContext *,uint16_t,uint16_t);
void jx_world_shift_pages(JxEventContext *,uint16_t,uint16_t);
void jx_world_moving_hazards(JxEventContext *);
uint16_t jx_cd_validate_track_range(JxEventContext *,JxFarPointer);
typedef struct {uint16_t address,count,page;} JxSoundDmaPorts;
void jx_sound_dma_setup(JxEventContext *,uint32_t,JxFarPointer,uint16_t,JxSoundDmaPorts);
typedef struct {uint8_t version;uint16_t command;} JxSoundDspScratch;
void jx_sound_dsp_write(JxEventContext *,uint16_t,uint16_t);
void jx_sound_dsp_start(JxEventContext *,uint16_t,JxFarPointer,uint16_t,JxSoundDspScratch);
typedef struct {JxEngineBridge base;JxSoundDmaPorts dma_stack;JxSoundDspScratch dsp_stack;} JxSoundBridge;
void jx_sound_bridge_install(JxEventContext *,JxSoundBridge *,JxSoundDmaPorts,JxSoundDspScratch);
void jx_sound_bridge_remove(JxEventContext *,JxSoundBridge *);
/* VOC work reserves 46 bytes and retains original header/status initial bytes. */
uint8_t jx_voc_load(JxEventContext *,JxFarPointer,uint16_t,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer work;} JxVocBridge;
void jx_voc_bridge_install(JxEventContext *,JxVocBridge *,JxFarPointer);
void jx_voc_bridge_remove(JxEventContext *,JxVocBridge *);
uint16_t jx_two_choice_menu(JxEventContext *,uint16_t,uint16_t,JxFarPointer,JxFarPointer,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer event_work,load_work;} JxMenuBridge;
void jx_menu_bridge_install(JxEventContext *,JxMenuBridge *,JxFarPointer,JxFarPointer);
void jx_menu_bridge_remove(JxEventContext *,JxMenuBridge *);
uint16_t jx_load_game_menu(JxEventContext *,JxFarPointer);
void jx_chapter_text_load(JxEventContext *,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer work;} JxChapterBridge;
void jx_chapter_bridge_install(JxEventContext *,JxChapterBridge *,JxFarPointer);
void jx_chapter_bridge_remove(JxEventContext *,JxChapterBridge *);
typedef enum {JX_BULK_WRITE_RETURNED=0,JX_BULK_DIVIDE_ERROR=1} JxBulkWriteFlow;
typedef struct {JxBulkWriteFlow flow;uint16_t value;} JxBulkWriteResult;
JxBulkWriteResult jx_stream_write_chunk(JxEventContext *,uint16_t,JxFarPointer,uint16_t);
JxBulkWriteResult jx_fwrite(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t);
void jx_save_game_menu(JxEventContext *,JxFarPointer);
void jx_shutdown_sound_storage(JxEventContext *);
void jx_shutdown_keyboard_vector(JxEventContext *);
void jx_shutdown_timer_callbacks(JxEventContext *);
void jx_shutdown_text_mode(JxEventContext *);
void jx_game_shutdown(JxEventContext *);
void jx_graphics_initialize(JxEventContext *);
void jx_configuration_load(JxEventContext *);
void jx_game_database_load(JxEventContext *);
void jx_new_game_initialize(JxEventContext *);
void jx_scene_presence_load(JxEventContext *);
typedef uint16_t (*JxBiosTimerLow)(void *);
typedef enum {JX_BOOT_MEMORY_RETURNED=0,JX_BOOT_MEMORY_EXIT=1} JxBootMemoryFlow;
JxBootMemoryFlow jx_boot_memory_initialize(JxEventContext *,JxBiosTimerLow,void *);
void jx_game_initialize(JxEventContext *);
uint16_t jx_game_entry(JxEventContext *);
typedef enum {JX_GAME_LOOP_EMPTY=0,JX_GAME_LOOP_PROCESSED=1} JxGameLoopStep;
void jx_game_loop_initialize(JxEventContext *,JxFarPointer);
JxGameLoopStep jx_game_loop_step(JxEventContext *,JxFarPointer);
void jx_game_main_loop(JxEventContext *,JxFarPointer);
typedef void (*JxCleanupInterruptEnable)(void *,uint8_t);
typedef void (*JxCleanupBiosVideo)(void *,uint16_t);
typedef struct {void *opaque;JxCleanupInterruptEnable interrupt_enable;JxCleanupBiosVideo bios_video;} JxCleanupDevice;
void jx_keyboard_restore_vector(JxEventContext *);
void jx_timer_restore_vector(JxEventContext *);
void jx_timer_remove_callback(JxEventContext *,uint16_t);
void jx_bios_video_call(const JxCleanupDevice *,uint16_t);
void jx_sound_restore_irq(JxEventContext *,JxFarPointer,const JxCleanupDevice *);
uint16_t jx_standard_stream_prepare(JxEventContext *,uint16_t);
void jx_standard_stream_finish(JxEventContext *,uint16_t,uint16_t);
uint16_t jx_printf_words(JxEventContext *,JxFarPointer,JxFarPointer);
uint16_t jx_printf_format(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer);
typedef void (*JxDosVectorService)(void *,uint8_t,JxFarPointer);
typedef uint32_t (*JxDosGetVectorService)(void *,uint8_t);
typedef void (*JxKeyboardBiosService)(void *,uint16_t,uint16_t);
uint32_t jx_dos_get_vector(JxDosGetVectorService,void *,uint16_t);
void jx_keyboard_install_vector(JxEventContext *);
void jx_timer_install_vector(JxEventContext *);
uint16_t jx_timer_add_callback(JxEventContext *,uint16_t);
void jx_keyboard_initialize(JxEventContext *,JxKeyboardBiosService,void *);
void jx_timer_initialize(JxEventContext *);
void jx_keyboard_irq(JxEventContext *,const JxPortDevice *,void (*interrupt_disable)(void *),void *,JxFarPointer);
void jx_timer_irq(JxEventContext *,JxFarPointer);
typedef struct {
    void *opaque;
    JxKeyboardBiosService keyboard_bios;
    JxDosGetVectorService get_vector;
    JxDosVectorService set_vector;
    void (*interrupt_disable)(void *);
} JxInputHardwareDevice;
typedef struct {
    JxEngineBridge base;
    JxPortDevice ports;
    JxInputHardwareDevice device;
    JxFarPointer irq_event;
    uint16_t active;
} JxInputHardwareBridge;
void jx_input_hardware_bridge_install(JxEventContext *,JxInputHardwareBridge *,const JxPortDevice *,const JxInputHardwareDevice *,JxFarPointer);
void jx_input_hardware_bridge_remove(JxEventContext *,JxInputHardwareBridge *);
uint16_t jx_dos_set_vector(JxDosVectorService,void *,uint16_t,JxFarPointer);
void jx_crt_run_exit_callbacks(JxEventContext *,uint16_t,uint16_t);
uint16_t jx_crt_error_message(JxEventContext *,uint16_t);
void jx_crt_report_corruption(JxEventContext *);
uint16_t jx_crt_check_data(JxEventContext *);
/* Error hook may alter BX/DX/CX; the original ABI preserves DS. */
typedef void (*JxCrtErrorHook)(void *,uint16_t,uint16_t *,uint16_t *,uint16_t *);
typedef void (*JxCrtOverlayCleanup)(void *,JxFarPointer,uint16_t,uint16_t);
typedef void (*JxCrtTerminate)(void *,uint8_t);
typedef struct {void *opaque;JxCrtOverlayCleanup overlay_cleanup;JxDosVectorService set_vector;JxCrtTerminate terminate;} JxCrtExitDevice;
typedef enum {JX_CRT_EXIT_RETURNED=0,JX_CRT_EXIT_TERMINATED=1} JxCrtExitFlow;
typedef struct {JxCrtExitFlow flow;uint16_t status;} JxCrtExitResult;
uint16_t jx_crt_write_error(JxEventContext *,uint16_t,JxDosWriteService,JxCrtErrorHook,void *);
void jx_crt_restore_system(JxEventContext *,const JxCrtExitDevice *);
JxCrtExitResult jx_crt_exit(JxEventContext *,uint16_t,uint16_t,const JxCrtExitDevice *);
JxCrtExitResult jx_crt_exit_quick(JxEventContext *,uint16_t,const JxCrtExitDevice *);
JxCrtExitResult jx_crt_cleanup(JxEventContext *,const JxCrtExitDevice *);
JxCrtExitResult jx_crt_cleanup_quick(JxEventContext *,const JxCrtExitDevice *);
uint16_t jx_stream_flush_all_worker(JxEventContext *,uint16_t);
uint16_t jx_stream_flush_all(JxEventContext *);
uint16_t jx_fflush(JxEventContext *,JxFarPointer);
uint16_t jx_stream_close_extra(JxEventContext *);
void jx_crt_stdio_cleanup(JxEventContext *);
uint16_t jx_dos_commit(JxEventContext *,uint16_t,JxDosCloseService,void *);
uint16_t jx_file_commit(JxEventContext *,uint16_t);
typedef void (*JxRegisteredEmsRelease)(void *,uint16_t);
typedef void (*JxRegisteredXmsCall)(void *,JxFarPointer,uint8_t,uint16_t);
typedef struct {void *opaque;JxDosVectorService set_vector;JxRegisteredEmsRelease release_ems;JxRegisteredXmsCall xms;} JxRegisteredCleanupDevice;
void jx_graphics_release_ems(JxEventContext *,const JxRegisteredCleanupDevice *);
void jx_graphics_release_xms(JxEventContext *,const JxRegisteredCleanupDevice *);
void jx_graphics_exit_vectors(JxEventContext *,const JxRegisteredCleanupDevice *);
void jx_graphics_restore_bios_flag(JxEventContext *);
typedef enum {JX_STORY_ANIMATION_RETURNED=0,JX_STORY_ANIMATION_DIVIDE_ERROR=1} JxStoryAnimationFlow;
JxStoryAnimationFlow jx_story_animation_play(JxEventContext *,JxFarPointer,uint16_t,uint16_t);
uint16_t jx_battle_run(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer);
void jx_battle_load_background(JxEventContext *,uint16_t,JxFarPointer);
void jx_battle_load_actor_image(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
void jx_battle_draw_animation_frame(JxEventContext *,uint16_t,uint16_t);
void jx_battle_draw_actor_pose(JxEventContext *,uint16_t);
void jx_battle_cache_actor_animation(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_battle_cache_animation(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
void jx_battle_award_experience(JxEventContext *,JxFarPointer);
void jx_battle_show_damage(JxEventContext *,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_action_menu(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer,JxFarPointer);
uint16_t jx_menu_select(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer,JxFarPointer,uint16_t *);
uint16_t jx_skill_menu(JxEventContext *,uint16_t,JxFarPointer);
void jx_skill_menu_render(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer);
uint16_t jx_item_menu(JxEventContext *,JxFarPointer);
typedef struct {char lines[7][32];unsigned count;} JxItemDescription;
void jx_item_description(JxEventContext *,uint16_t,JxItemDescription *);
void jx_item_menu_render(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer);
void jx_actor_menu_show(JxEventContext *,uint16_t,JxFarPointer);
enum {
    JX_BATTLE_CONTROL=1,JX_BATTLE_MENUS=2,JX_BATTLE_IMAGES=4,
    JX_BATTLE_DRAW=8,JX_BATTLE_CACHE=16,JX_BATTLE_SETTLEMENT=32,
    JX_BATTLE_INVENTORY=64,JX_BATTLE_INPUT=128,JX_BATTLE_ALL=255,
    JX_BATTLE_WORK_BYTES=614
};
typedef struct {JxEngineBridge base;JxFarPointer work;uint16_t features;uint32_t active;} JxBattleBridge;
/* work reserves614 contiguous disjoint bytes. Input producers must fill
 * the ring externally when INPUT is enabled. Remove bridges in reverse. */
void jx_battle_bridge_install(JxEventContext *,JxBattleBridge *,JxFarPointer,uint16_t);
void jx_battle_bridge_remove(JxEventContext *,JxBattleBridge *);
uint16_t jx_chance_percent(JxEventContext *,uint16_t);
void jx_battle_run_turn(JxEventContext *,JxFarPointer);
void jx_battle_turn_dispatch(JxEventContext *);
void jx_battle_choose_attack(JxEventContext *);
void jx_battle_open_actor_menu(JxEventContext *);
void jx_battle_choose_item(JxEventContext *);
void jx_battle_choose_skill(JxEventContext *);
/* DOS calendar registers are supplied by the host; no host interrupt executes. */
typedef struct JxDosCalendarRegisters { uint16_t ax,cx,dx; } JxDosCalendarRegisters;
typedef void (*JxDosCalendarService)(void *,uint8_t,JxDosCalendarRegisters *);
uint16_t jx_dos_get_date(JxEventContext *,JxFarPointer,JxDosCalendarService,void *);
uint16_t jx_dos_get_time(JxEventContext *,JxFarPointer,JxDosCalendarService,void *);
uint32_t jx_clock_milliseconds(JxEventContext *,JxFarPointer);
void jx_wait_milliseconds(JxEventContext *,uint16_t);
/* private_work reserves74 logical bytes plus a physical spill byte when a
 * word starts atFFFF; disjoint from caller data. Bridge is not reentrant. */
typedef struct {JxEngineBridge base;JxDosCalendarService service;void *service_opaque;JxFarPointer private_work;} JxCalendarBridge;
void jx_calendar_bridge_install(JxEventContext *,JxCalendarBridge *,JxDosCalendarService,void *,JxFarPointer);
void jx_calendar_bridge_remove(JxEventContext *,JxCalendarBridge *);
typedef struct {JxEngineBridge base;JxInputPoll poll;void *poll_opaque;uint16_t active;} JxInputWaitBridge;
void jx_input_wait_bridge_install(JxEventContext *,JxInputWaitBridge *,JxInputPoll,void *);
void jx_input_wait_bridge_remove(JxEventContext *,JxInputWaitBridge *);
typedef struct {JxEngineBridge base;JxPortDevice device;} JxPortBridge;
void jx_port_bridge_install(JxEventContext *,JxPortBridge *,const JxPortDevice *);
void jx_port_bridge_remove(JxEventContext *,JxPortBridge *);
void jx_memory_copy_fixed(JxEventContext *,JxFarPointer,JxFarPointer,uint16_t);
void jx_npc_erase_transition(JxEventContext *,uint16_t);
void jx_npc_erase_background(JxEventContext *,uint16_t);
typedef struct {JxEngineBridge base;uint16_t active;} JxNpcEraseBridge;
void jx_npc_erase_bridge_install(JxEventContext *,JxNpcEraseBridge *);
void jx_npc_erase_bridge_remove(JxEventContext *,JxNpcEraseBridge *);
uint16_t jx_actor_skill_grant(JxEventContext *,uint16_t,uint16_t);
void jx_animation_play_fixed_delay(JxEventContext *,JxFarPointer,uint16_t,uint16_t);
typedef struct {JxEngineBridge base;uint16_t active;} JxFixedAnimationBridge;
void jx_animation_fixed_bridge_install(JxEventContext *,JxFixedAnimationBridge *);
void jx_animation_fixed_bridge_remove(JxEventContext *,JxFixedAnimationBridge *);
void jx_scene_run_trajectories(JxEventContext *);
uint16_t jx_getc(JxEventContext *,uint16_t stream);
uint16_t jx_getc_dos(JxEventContext *,uint16_t,const JxReadServices *);
uint16_t jx_choice_menu(JxEventContext *,uint16_t,uint16_t,JxFarPointer,JxFarPointer,JxFarPointer,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer event;uint16_t active;} JxChoiceBridge;
void jx_choice_bridge_install(JxEventContext *,JxChoiceBridge *,JxFarPointer);
void jx_choice_bridge_remove(JxEventContext *,JxChoiceBridge *);
uint32_t jx_module_multiply_u32(uint32_t,uint32_t);
int jx_divide_unsigned32(uint32_t,uint32_t,uint32_t *);
int jx_remainder_unsigned32(uint32_t,uint32_t,uint32_t *);
void jx_dice_reorder(JxEventContext *,JxFarPointer,JxFarPointer,JxFarPointer);
uint16_t jx_dice_choose_wager(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer);
void jx_dice_play_animation(JxEventContext *,JxFarPointer,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer work;uint16_t active;} JxDiceBridge;
void jx_dice_bridge_install(JxEventContext *,JxDiceBridge *,JxFarPointer);
void jx_dice_bridge_remove(JxEventContext *,JxDiceBridge *);
uint16_t jx_dice_game(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer);
typedef struct {JxEngineBridge base;JxFarPointer frame;uint16_t active;} JxDiceGameBridge;
void jx_dice_game_bridge_install(JxEventContext *,JxDiceGameBridge *,JxFarPointer);
void jx_dice_game_bridge_remove(JxEventContext *,JxDiceGameBridge *);
JxTextResult jx_font_draw_text_timed(JxEventContext *,uint16_t,uint16_t,JxFarPointer,uint16_t);
int jx_text_pages(JxEventContext *,uint16_t,uint16_t,uint16_t,JxFarPointer,uint16_t,JxFarPointer);
typedef struct {uint16_t ax,bx,cx,dx,si,di,es,flags;} JxLegacyBiosRegisters;
typedef void (*JxLegacyBiosService)(void *,JxLegacyBiosRegisters *);
typedef struct {void *opaque;JxLegacyBiosService bios;} JxLegacyVideoDevice;
uint16_t jx_legacy_video_probe_vesa(JxEventContext *,const JxLegacyVideoDevice *,JxFarPointer);
uint16_t jx_legacy_video_check_display(JxEventContext *);
void jx_ending_animation(JxEventContext *,JxFarPointer,JxFarPointer);

void jx_legacy_video_viewport_center(JxEventContext *);
void jx_legacy_video_reset_modes(JxEventContext *);
void jx_legacy_video_planar_address(JxEventContext *,uint16_t,uint16_t);
uint16_t jx_legacy_video_planar_left(JxEventContext *);
uint16_t jx_legacy_video_planar_up(JxEventContext *,uint16_t);
uint16_t jx_legacy_video_planar_down(JxEventContext *,uint16_t);
void jx_legacy_video_planar_color(JxEventContext *,uint16_t);
void jx_legacy_video_reset_registers(JxEventContext *,const JxVideoBus *);
uint16_t jx_legacy_video_planar_read(JxEventContext *,const JxVideoBus *);
void jx_legacy_video_planar_write(JxEventContext *,const JxVideoBus *);
void jx_legacy_video_text_page_offsets(JxEventContext *);
void jx_legacy_video_text_pages(JxEventContext *);
uint16_t jx_legacy_video_select_rows(JxEventContext *);
typedef void (*JxLegacyNearService)(void *,uint16_t,uint16_t);
void jx_legacy_video_load_descriptor(JxEventContext *,uint16_t,uint16_t,JxLegacyNearService,void *);
uint16_t jx_legacy_video_probe_olivetti(JxEventContext *);
uint16_t jx_legacy_video_check_mode(JxEventContext *,const JxLegacyVideoDevice *);
void jx_legacy_video_update_equipment(JxEventContext *,const JxLegacyVideoDevice *);
void jx_legacy_video_detect_adapter(JxEventContext *,const JxLegacyVideoDevice *,const JxVideoBus *,JxFarPointer);
uint16_t jx_legacy_video_setup_text_mode(JxEventContext *,uint16_t,JxLegacyNearService,void *);
uint16_t jx_legacy_video_initialize_font_rows(JxEventContext *,const JxLegacyVideoDevice *,uint16_t,uint16_t *);
uint16_t jx_legacy_video_scale_cursor(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t *);
uint16_t jx_legacy_video_set_cursor_shape(JxEventContext *,uint16_t,uint16_t *);
uint16_t jx_legacy_video_unscale_cursor(JxEventContext *,uint16_t,uint16_t *);
void jx_legacy_video_capture_state(JxEventContext *,const JxLegacyVideoDevice *,const JxVideoBus *,JxFarPointer);
void jx_legacy_video_compose_color(JxEventContext *,JxLegacyNearService,void *);
uint16_t jx_legacy_video_set_foreground(JxEventContext *,uint16_t,JxLegacyNearService,void *);
void jx_legacy_video_reset_text_state(JxEventContext *,JxLegacyNearService,void *);
uint16_t jx_legacy_video_initialize_text(JxEventContext *,const JxLegacyVideoDevice *,const JxVideoBus *,JxFarPointer);
typedef void (*JxLegacyPointService)(void *,uint16_t,uint16_t,uint16_t);
void jx_legacy_video_clip_addresses(JxEventContext *,uint16_t,uint16_t,uint16_t,uint16_t,JxLegacyPointService,void *);
void jx_legacy_video_reset_viewport(JxEventContext *,JxLegacyPointService,void *);
void jx_legacy_video_reset_view(JxEventContext *,JxLegacyPointService,void *);
void jx_legacy_video_reset_state(JxEventContext *,JxLegacyNearService,JxLegacyPointService,void *);
typedef uint16_t (*JxLegacyPaletteService)(void *,uint16_t,JxFarPointer);
uint16_t jx_legacy_video_select_cga_palette(JxEventContext *,uint16_t,const JxLegacyVideoDevice *,JxLegacyPaletteService,void *);
void jx_legacy_video_palette_block(JxEventContext *,const JxLegacyVideoDevice *,JxFarPointer,JxFarPointer);
uint16_t jx_legacy_video_set_palette(JxEventContext *,JxFarPointer,JxLegacyPaletteService,void *);
uint16_t jx_legacy_video_select_font(JxEventContext *,uint16_t,uint16_t *,uint16_t *);
void jx_legacy_video_activate_planar(JxEventContext *,const JxLegacyVideoDevice *,JxLegacyBiosRegisters *);
uint16_t jx_legacy_video_select_planar_page(JxEventContext *,uint16_t);
void jx_legacy_video_initialize_palette(JxEventContext *,const JxLegacyVideoDevice *);
uint16_t jx_legacy_video_setup_mode18(JxEventContext *,JxLegacyNearService,void *);
typedef void (*JxLegacyBankService)(void *,JxFarPointer,uint16_t,uint16_t);
void jx_legacy_image_capture_scanline(JxEventContext *,const JxVideoBus *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer *,JxFarPointer *,JxLegacyBankService,void *);
void jx_legacy_image_prepare_write(JxEventContext *,const JxVideoBus *,uint16_t);
uint16_t jx_legacy_image_put_scanline(JxEventContext *,const JxVideoBus *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer *,JxFarPointer *,JxLegacyBankService,void *);
uint16_t jx_legacy_image_setup(JxEventContext *,const JxVideoBus *,JxFarPointer,uint16_t,uint16_t,uint16_t);
uint16_t jx_legacy_image_capture_row(JxEventContext *,const JxVideoBus *,JxLegacyBankService,void *);
uint16_t jx_legacy_image_put_row(JxEventContext *,const JxVideoBus *,JxLegacyBankService,void *);
uint16_t jx_legacy_image_contains(JxEventContext *,uint16_t,uint16_t);
uint16_t jx_legacy_image_translate_point(JxEventContext *,uint16_t,uint16_t);
int jx_legacy_image_capture(JxEventContext *,const JxVideoBus *,uint16_t,uint16_t,uint16_t,uint16_t,JxFarPointer,JxLegacyPointService,JxLegacyNearService,JxLegacyBankService,void *);
int jx_legacy_image_put(JxEventContext *,const JxVideoBus *,uint16_t,uint16_t,JxFarPointer,uint16_t,JxLegacyPointService,JxLegacyNearService,JxLegacyBankService,void *);
int jx_legacy_image_enter_mode18(JxEventContext *,const JxLegacyVideoDevice *,JxLegacyNearService,JxLegacyPointService,void *,uint16_t *);
void jx_board_quest(JxEventContext *,uint16_t);
void jx_callback_2f28_2d9c(JxEventContext *,uint16_t);
void jx_callback_1f2a_de00(JxEventContext *);
void jx_callback_1f2a_ec30(JxEventContext *);
void jx_callback_1f2a_ee48(JxEventContext *);
void jx_callback_1f2a_f912(JxEventContext *);
void jx_callback_1f2a_fece(JxEventContext *);
void jx_callback_2f28_013a(JxEventContext *,uint16_t);
void jx_callback_2f28_1942(JxEventContext *);
void jx_callback_2f28_064e(JxEventContext *,uint16_t);
void jx_callback_2f28_308a(JxEventContext *);
void jx_callback_1f2a_e69a(JxEventContext *,uint16_t);
void jx_callback_1f2a_f108(JxEventContext *c);
void jx_callback_1f2a_f150(JxEventContext *c);
void jx_callback_1f2a_f3a4(JxEventContext *c);
void jx_callback_2f28_09b4(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0334(JxEventContext *c);
void jx_callback_2f28_07c6(JxEventContext *c);
void jx_callback_1f2a_ea1a(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0a1c(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_122e(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_12be(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0de0(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0e54(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_15f0(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_2f44(JxEventContext *c);
void jx_callback_2f28_304c(JxEventContext *c);
void jx_callback_2f28_141a(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_238a(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_23d2(JxEventContext *c);
void jx_callback_2f28_2482(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_24f6(JxEventContext *c,uint16_t npc);
void jx_callback_1f2a_f20e(JxEventContext *c);
void jx_callback_1f2a_f822(JxEventContext *c);
void jx_callback_1f2a_f950(JxEventContext *c);
void jx_callback_2f28_149a(JxEventContext *c);
void jx_callback_2f28_2eb4(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_30d0(JxEventContext *c);
void jx_callback_2f28_1734(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_17d0(JxEventContext *c);
void jx_callback_2f28_18ec(JxEventContext *c);
void jx_callback_1f2a_fa0e(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_1588(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0014(JxEventContext *c);
void jx_callback_2f28_047c(JxEventContext *c);
void jx_callback_2f28_03d0(JxEventContext *c);
void jx_callback_1f2a_f460(JxEventContext *c);
void jx_callback_1f2a_f648(JxEventContext *c);
void jx_callback_2f28_0526(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_086a(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0c78(JxEventContext *c);
void jx_callback_2f28_0fd6(JxEventContext *c);
void jx_callback_2f28_1144(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_1356(JxEventContext *c,uint16_t npc);
void jx_callback_1f2a_fbe0(JxEventContext *c);
void jx_callback_1f2a_fd28(JxEventContext *c,uint16_t npc);
void jx_callback_1f2a_fe54(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_1b68(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_1e5e(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_228a(JxEventContext *c);
void jx_callback_2f28_2072(JxEventContext *c,uint16_t npc);
void jx_callback_1f2a_dd20(JxEventContext *c);
void jx_callback_2f28_1d32(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_331c(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_32d8(JxEventContext *c,uint16_t npc);
void jx_callback_2f28_0b22(JxEventContext *c);
void jx_callback_2f28_1fc4(JxEventContext *c,uint16_t npc);
void jx_cinematic_backdrop(JxEventContext *c,uint16_t frame);
void jx_cinematic_scroll(JxEventContext *c,uint16_t position);
void jx_callback_1f2a_eaac(JxEventContext *c);
void jx_callback_1f2a_f72e(JxEventContext *);
void jx_callback_2f28_11c4(JxEventContext *,uint16_t);
void jx_callback_2f28_01a2(JxEventContext *,uint16_t);
void jx_callback_2f28_06e4(JxEventContext *,uint16_t);
void jx_callback_2f28_0aa0(JxEventContext *,uint16_t);
void jx_callback_2f28_2946(JxEventContext *);
void jx_callback_2f28_2a46(JxEventContext *);
void jx_callback_2f28_2aee(JxEventContext *);
/* Original1832:3100/3127; optional cursor/render services stay explicit. */
uint16_t jx_legacy_image_begin(JxEventContext *c);
void jx_legacy_image_end(JxEventContext *c);
#endif
