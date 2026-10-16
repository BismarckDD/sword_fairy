/* Recovered from MAIN.EXE, not original source. All disk integers are LE.
 * Evidence addresses are linear addresses in save_tool/memory.bin,
 * loaded at 1000:0000. The original EXE is packed; these are NOT file offsets.
 */
#ifndef JX_SAVE_FORMAT_H
#define JX_SAVE_FORMAT_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define JX_SAVE_SIZE 17266u
#pragma pack(push, 1)
typedef struct {
    char name_gbk[12];                 /* +00, NUL-terminated GBK/GB2312 */
    int16_t unknown_0c, unknown_0e;
    int16_t hp, mp, max_hp, max_mp;     /* +10,+12,+14,+16 */
    int16_t base_attack, base_defense;  /* +18,+1a */
    int16_t unknown_1c;
    uint16_t experience;               /* +1e: displayed as one WORD */
    int16_t unknown_20;
    int16_t level;                     /* +22 */
    int16_t condition;                 /* +24: 1=drunk, 2=poison */
    int16_t equipment_id;              /* +26: THING.DAT index */
    int16_t unknown_28, unknown_2a, unknown_2c;
    int16_t unknown_2e, unknown_30, unknown_32;
    uint8_t skill_ids[16];             /* +34; 0xff = unused */
} JxActor;

typedef struct { uint16_t item_id, count; } JxInventorySlot;
typedef struct { int16_t x, y, direction; } JxPathPoint;
typedef struct {
    int16_t unknown_00, unknown_02;    /* do not assume these are money */
    int32_t money;                    /* +04: paired ADD/ADC and SUB/SBB */
    int16_t party[3];                 /* +08: actor indices, -1 = absent */
    JxPathPoint path[6];              /* +0e: movement/path samples */
    int16_t unknown_32[3];
    JxInventorySlot inventory[64];    /* +38 */
} JxGlobals;

typedef struct {
    int16_t x, y, unknown_04;         /* memory NPC +40..+45 */
    int16_t unknown_06;               /* memory NPC +3c..+3d */
} JxNpcSaved;

typedef struct {
    int16_t chapter;                  /* DS:8894; constructs chp%d.txt */
    int16_t music_id;                 /* DS:889a; -1 allowed */
    JxActor actors[16];               /* 0004..0443 */
    JxGlobals globals;                /* 0444..057b */
    int16_t scene_id;                 /* 057c..057d */
    uint8_t scene_flags[220];         /* 057e..0659 */
    uint8_t npc_flags_a[220][30];      /* 065a..2021 */
    uint8_t npc_flags_b[220][30];      /* 2022..39e9 */
    uint8_t object_flags[220][10];     /* 39ea..4281 */
    JxNpcSaved npcs[30];              /* 4282..4371 */
} JxSave;
#pragma pack(pop)

_Static_assert(sizeof(JxActor)==68, "actor layout");
_Static_assert(sizeof(JxGlobals)==312, "globals layout");
_Static_assert(sizeof(JxSave)==JX_SAVE_SIZE, "save layout");
_Static_assert(offsetof(JxSave, globals)==0x444, "globals offset");
_Static_assert(offsetof(JxSave, npcs)==0x4282, "NPC offset");

/* Portable byte-level translations of the original fread/fwrite sequence. */
int jx_read_save(FILE *fp, uint8_t ds[65536]);
int jx_write_save(FILE *fp, const uint8_t ds[65536]);
int jx_display_attack(const uint8_t ds[65536], unsigned actor);
int jx_display_defense(const uint8_t ds[65536], unsigned actor);
void jx_compact_inventory(JxSave *s);
int jx_has_item(const JxSave *s, int16_t item_id);
void jx_grant_item_core(JxSave *s, int16_t item_id, int16_t count);
void jx_remove_item(JxSave *s, int16_t item_id, int16_t count);
void jx_compact_skills(JxActor *actor);
void jx_remove_skill(JxActor *actor, int16_t skill_id);
void jx_restore_actor(JxActor *actor);
void jx_level_up_core(JxSave *save, int actor_index);
#endif
