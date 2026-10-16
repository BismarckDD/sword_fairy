/* Build: zig cc -std=c11 -Wall -Wextra save_io.c inventory.c verify.c -o verify.exe */
#include "save_format.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(int argc, char **argv) {
    uint8_t ds[65536], original[JX_SAVE_SIZE], result[JX_SAVE_SIZE];
    for (int i=1; i<argc; ++i) {
        FILE *f=fopen(argv[i],"rb"); CHECK(f);
        CHECK(fread(original,1,sizeof original,f)==sizeof original);
        CHECK(fgetc(f)==EOF); rewind(f); memset(ds,0xcc,sizeof ds);
        CHECK(jx_read_save(f,ds)==0); fclose(f);
        FILE *out=tmpfile(); CHECK(out);
        CHECK(jx_write_save(out,ds)==0); CHECK(ftell(out)==JX_SAVE_SIZE);
        rewind(out); CHECK(fread(result,1,sizeof result,out)==sizeof result); fclose(out);
        CHECK(memcmp(original,result,sizeof original)==0);
        printf("PASS byte-for-byte C roundtrip: %s\n",argv[i]);
    }
    JxSave s; memset(&s,0,sizeof s);
    s.globals.inventory[0]=(JxInventorySlot){2,3};
    s.globals.inventory[1]=(JxInventorySlot){9,0};
    s.globals.inventory[7]=(JxInventorySlot){4,5};
    s.globals.inventory[9]=(JxInventorySlot){5,65535};
    CHECK(jx_has_item(&s,9)==1); /* intentionally ignores zero quantity */
    jx_compact_inventory(&s);
    CHECK(s.globals.inventory[0].item_id==2 && s.globals.inventory[1].item_id==4);
    CHECK(s.globals.inventory[2].item_id==0);
    jx_grant_item_core(&s,2,8); CHECK(s.globals.inventory[0].count==11);
    jx_grant_item_core(&s,6,2); CHECK(s.globals.inventory[2].item_id==6);
    jx_remove_item(&s,2,12); CHECK(s.globals.inventory[0].item_id==4);
    for (int i=0;i<64;++i) s.globals.inventory[i]=(JxInventorySlot){(uint16_t)(i+1),1};
    jx_grant_item_core(&s,70,2); CHECK(s.globals.inventory[63].item_id==70);
    memset(s.actors[0].skill_ids,255,16);
    s.actors[0].skill_ids[1]=4;s.actors[0].skill_ids[8]=128;s.actors[0].skill_ids[12]=7;
    jx_compact_skills(&s.actors[0]);
    CHECK(s.actors[0].skill_ids[0]==4 && s.actors[0].skill_ids[1]==7 && s.actors[0].skill_ids[2]==255);
    jx_remove_skill(&s.actors[0],4); CHECK(s.actors[0].skill_ids[0]==7);
    s.actors[0].level=30;s.actors[0].max_hp=1285;s.actors[0].max_mp=810;
    s.actors[0].base_attack=504;s.actors[0].base_defense=494;s.actors[0].condition=2;
    jx_level_up_core(&s,0);
    CHECK(s.actors[0].level==31 && s.actors[0].max_hp==1345 && s.actors[0].hp==672);
    CHECK(s.actors[0].max_mp==850 && s.actors[0].mp==425 && s.actors[0].condition==2);
    CHECK(s.actors[0].base_attack==528 && s.actors[0].base_defense==518);
    puts("PASS inventory/skill/level-up edge cases and structure sizes");
    return 0;
}
