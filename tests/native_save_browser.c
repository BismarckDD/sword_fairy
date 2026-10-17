/* Isolated persistence and game-native browser input regression.
 * Run in a fresh directory; never point the test at player userdata. */
#include <assert.h>
#include "../src/platform/windows/native_saves.c"
#include "save_format.h"
static JxNativeWindow window;
static JxVideoModel video;
static uint8_t memory[1048576],restored[65536],palette[768];
static uint8_t *ds=memory+0x39e30;
static const uint16_t (*events)[2];static unsigned event_count,event_index;
static JxEventContext context;
void jx_native_window_present(JxNativeWindow *w){(void)w;}
void jx_native_window_sync_ticks(JxNativeWindow *w){(void)w;}
static uint32_t dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    (void)opaque;
    if(segment==0x1f2a && offset==0xc1f2 && n==2) {
        assert(event_index<event_count);JxFarPointer input={a[0],a[1]};
        jx_far_set_byte(&context,input,0,(uint8_t)events[event_index][0]);jx_far_set_byte(&context,input,1,0);
        jx_far_set_byte(&context,input,2,(uint8_t)events[event_index][1]);jx_far_set_byte(&context,input,3,0);event_index++;return 0;
    }
    if(segment==0x1832 && offset==0x1810){context.memory[0x44e]=0;context.memory[0x44f]=(uint8_t)(a[0]*0x40);}
    return 0;
}
static void input(const uint16_t (*script)[2],unsigned count) {events=script;event_count=count;event_index=0;}
int main(void) {
    assert(GetFileAttributesA("main_decompiled")==INVALID_FILE_ATTRIBUTES);
    assert(_mkdir("main_decompiled")==0);assert(_mkdir("main_decompiled/userdata")==0);
    for(unsigned i=0;i<65536;i++)ds[i]=(uint8_t)(i*37);ds[0x9334]=5;ds[0x9335]=0;
    window.video=&video;window.palette=palette;window.save_scene_valid=1;
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++)window.pixels[y*320+x]=(x/2)*0x10000+(y/2)*0x100;
    assert(jx_native_save_browser(&window,ds,2)==0);
    assert(window.save_scene[99*160+159]==159*0x10000+99*0x100);
    Browser b={0};b.ds=ds;b.window=&window;int slots[]={0,9,10,98};
    for(unsigned i=0;i<4;i++){
        b.selected=slots[i];assert(commit_save(&b));inspect(&b,b.selected);assert(b.slots[b.selected].state==1 && b.slots[b.selected].preview);
        char path[160];path_for(path,sizeof path,b.selected,"DAT");FILE *f=fopen(path,"rb");assert(f);memset(restored,0,sizeof restored);assert(jx_read_save(f,restored)==0);assert(fgetc(f)==EOF);fclose(f);
        assert(memcmp(restored+0xd04e,ds+0xd04e,1088)==0);assert(memcmp(restored+0x665a,ds+0x665a,6600)==0);assert(memcmp(restored+0x889c+64,ds+0x889c+64,6)==0);
    }
    /* A failed final replacement must not report success or lose other slots. */
    assert(_mkdir("main_decompiled/userdata/SAVE21.DAT")==0);b.selected=21;assert(!commit_save(&b));inspect(&b,0);assert(b.slots[0].state==1 && b.slots[0].preview);
    inspect(&b,1);assert(b.slots[1].state==0);
    assert(DeleteFileA("main_decompiled/userdata/SAVE10.preview"));inspect(&b,10);assert(b.slots[10].state==1 && !b.slots[10].preview);
    FILE *f=fopen("main_decompiled/userdata/SAVE1.DAT","wb");assert(f);fputs("broken",f);fclose(f);inspect(&b,1);assert(b.slots[1].state==2);
    /* A mismatching preview must never be displayed for another save. */
    f=fopen("main_decompiled/userdata/SAVE9.DAT","r+b");assert(f);fputc(0,f);fclose(f);inspect(&b,9);assert(b.slots[9].state==1 && !b.slots[9].preview);
    /* Legacy root saves remain visible when no userdata counterpart exists. */
    assert(CopyFileA("main_decompiled/userdata/SAVE0.DAT","SAVE3.DAT",TRUE));inspect(&b,3);assert(b.slots[3].state==1 && !b.slots[3].preview);
    context=(JxEventContext){memory,0x39e3,dispatch,NULL,NULL,NULL};
    for(unsigned i=0;i<768;i++)palette[i]=(uint8_t)(i&63);
    for(unsigned p=0;p<4;p++)for(unsigned i=0;i<65536;i++)video.planes[p][i]=(uint8_t)(i+p);
    uint32_t before=hash_bytes((uint8_t *)video.planes,sizeof video.planes);
    static const uint16_t read99[][2]={{1,0x4b},{1,0x48},{0,0x39}};
    last_slot=0;input(read99,3);assert(jx_native_game_save_browser(&window,&context,0)==98 && event_index==3);
    assert(before==hash_bytes((uint8_t *)video.planes,sizeof video.planes));
    static const uint16_t save21[][2]={{0,0x39}};
    last_slot=20;input(save21,1);assert(jx_native_game_save_browser(&window,&context,1)==20);inspect(&b,20);assert(b.slots[20].state==1 && b.slots[20].preview);
    uint32_t old_hash=b.slots[20].hash;ds[0x8894]^=1;
    static const uint16_t cancel_overwrite[][2]={{0,0x1c},{0,0x39},{1,1}};
    input(cancel_overwrite,3);assert(jx_native_game_save_browser(&window,&context,1)==-1);inspect(&b,20);assert(b.slots[20].hash==old_hash);
    static const uint16_t accept_overwrite[][2]={{0,0x39},{1,0x48},{0,0x1c}};
    input(accept_overwrite,3);assert(jx_native_game_save_browser(&window,&context,1)==20);inspect(&b,20);assert(b.slots[20].hash!=old_hash && b.slots[20].preview);
    static const uint16_t wrap[][2]={{1,0x4d},{1,0x4b},{0,0x1c}};
    last_slot=98;input(wrap,3);assert(jx_native_game_save_browser(&window,&context,0)==98);
    static const uint16_t empty[][2]={{0,0x39},{1,1}};
    last_slot=30;input(empty,2);assert(jx_native_game_save_browser(&window,&context,0)==-1);
    last_slot=1;input(empty,2);assert(jx_native_game_save_browser(&window,&context,0)==-1);
    puts("PASS: original save restoration, slots 01/10/11/99, empty/corrupt/legacy files, matching previews, game-native paging, Enter/Space, overwrite default cancel, explicit overwrite, background restoration.");return 0;
}
