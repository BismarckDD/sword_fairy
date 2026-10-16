/* Manual C reconstruction of save/load core.
 * Original fwrite at 1000:08e8, fread at 1000:0754, fclose at 1000:0636.
 * Save core linear 29434..29582; load core 29979..29ac7.
 * UI/file-name assembly is omitted. Unlike the original, errors are checked.
 * DS is the game's 64 KiB data segment, not an operating-system pointer.
 */
#include "save_format.h"

typedef struct { uint16_t ds_offset, size; } Block;
static const Block blocks[] = {
    {0x8894, 2}, {0x889a, 2}, {0xd04e, 16*68},
    {0x8752, 0x138}, {0x9334, 2}, {0x3466, 220},
    {0x665a, 6600}, {0xb686, 6600}, {0xacec, 2200}
};

int jx_read_save(FILE *fp, uint8_t ds[65536]) {
    for (unsigned i=0; i<sizeof(blocks)/sizeof(blocks[0]); ++i)
        if (fread(ds+blocks[i].ds_offset, 1, blocks[i].size, fp)!=blocks[i].size)
            return -1;
    for (unsigned i=0; i<30; ++i) {
        unsigned npc=0x889c+i*0x56;
        if (fread(ds+npc+0x40, 1, 6, fp)!=6) return -1;
        if (fread(ds+npc+0x3c, 1, 2, fp)!=2) return -1;
    }
    return 0;
}

int jx_write_save(FILE *fp, const uint8_t ds[65536]) {
    for (unsigned i=0; i<sizeof(blocks)/sizeof(blocks[0]); ++i)
        if (fwrite(ds+blocks[i].ds_offset, 1, blocks[i].size, fp)!=blocks[i].size)
            return -1;
    for (unsigned i=0; i<30; ++i) {
        unsigned npc=0x889c+i*0x56;
        if (fwrite(ds+npc+0x40, 1, 6, fp)!=6) return -1;
        if (fwrite(ds+npc+0x3c, 1, 2, fp)!=2) return -1;
    }
    return 0;
}

static int s16(const uint8_t *p) {
    unsigned v=(unsigned)p[0] | ((unsigned)p[1]<<8);
    return v>=0x8000 ? (int)v-65536 : (int)v;
}

static int add16(int a, int b) {
    unsigned v=(unsigned)(a+b)&65535u;
    return v>=32768u ? (int)v-65536 : (int)v;
}

/* 27b2c..27b67: displayed attack includes equipment's +16 WORD. */
int jx_display_attack(const uint8_t ds[65536], unsigned actor) {
    if (actor>=16) return -1;
    unsigned a=0xd04e + actor*68;
    int equipment=s16(ds+a+0x26);
    if (equipment<0 || equipment>=64) return -1;
    return add16(s16(ds+a+0x18),s16(ds+0x8022+equipment*28+0x16));
}

/* 27b90..27bcb: displayed defense includes equipment's +18 WORD. */
int jx_display_defense(const uint8_t ds[65536], unsigned actor) {
    if (actor>=16) return -1;
    unsigned a=0xd04e + actor*68;
    int equipment=s16(ds+a+0x26);
    if (equipment<0 || equipment>=64) return -1;
    return add16(s16(ds+a+0x1a),s16(ds+0x8022+equipment*28+0x18));
}
