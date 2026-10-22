#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "native_window.h"
static JxNativeWindow window;
static JxVideoModel video;
static JxLegacyVgaModel legacy;
static uint8_t palette[768];
static uint16_t mode;
static uint8_t memory[1<<20];
static JxEventContext context;
static void tick(void *opaque){(void)opaque;}
static void scan(void *opaque,uint8_t value){(void)opaque;(void)value;}
int main(void){
    legacy.planar=&video;
    window.video=&video;window.legacy=&legacy;window.palette=palette;
    window.mode=&mode;window.tick=tick;window.scan=scan;
    assert(jx_native_window_open(&window));
    RECT client;assert(GetClientRect((HWND)window.handle,&client));
    printf("Client: %ld x %ld\n",client.right,client.bottom);fflush(stdout);
    assert(client.right==960 && client.bottom==600);
    /* After rollback the original presentation fills the client rectangle. */
    for(unsigned i=0;i<768;i++)palette[i]=63;
    RECT bounds={0,0,1000,600};assert(AdjustWindowRect(&bounds,WS_OVERLAPPEDWINDOW,FALSE));
    assert(SetWindowPos((HWND)window.handle,NULL,0,0,bounds.right-bounds.left,
        bounds.bottom-bounds.top,SWP_NOMOVE|SWP_NOZORDER));
    jx_native_window_present(&window);
    HDC dc=GetDC((HWND)window.handle);assert(dc);
    assert(GetPixel(dc,10,300)==RGB(252,252,252));
    assert(GetPixel(dc,500,300)==RGB(252,252,252));
    ReleaseDC((HWND)window.handle,dc);
    mode=18;jx_native_window_present(&window);assert(window.planar_paints>0);
    jx_native_window_close(&window);
    puts("PASS: restored 960x600 client, full-window pixels and planar mode repaint");
    mode=0;window.hd=jx_hd_open("build/hd-fixture");assert(window.hd);
    assert(jx_native_window_open(&window));assert(GetClientRect((HWND)window.handle,&client));
    assert(client.right==1280 && client.bottom==800);
    assert(SetWindowPos((HWND)window.handle,NULL,0,0,bounds.right-bounds.left,
        bounds.bottom-bounds.top,SWP_NOMOVE|SWP_NOZORDER));
    jx_native_window_present(&window);dc=GetDC((HWND)window.handle);assert(dc);
    assert(GetPixel(dc,10,300)==RGB(0,0,0));assert(GetPixel(dc,500,300)==RGB(252,252,252));
    ReleaseDC((HWND)window.handle,dc);
    assert(jx_native_window_post_scan(&window,0x42));assert(jx_native_window_post_scan(&window,0xc2));
    assert(jx_native_window_pump(&window));assert(window.hd_disabled);
    assert(jx_native_window_post_scan(&window,0x42));assert(jx_native_window_post_scan(&window,0xc2));
    assert(jx_native_window_pump(&window));assert(!window.hd_disabled);
    assert(SendMessageW((HWND)window.handle,WM_ERASEBKGND,0,0)==1);
    context.memory=memory;context.data_segment=0;
    jx_set_word(&context,0x499e + 30+14,500);
    jx_set_word(&context,0x6614,400);
    jx_set_word(&context,0xd04e + 16,100);jx_set_word(&context,0xd04e + 20,200);
    jx_set_word(&context,0xd04e + 68+16,70);jx_set_word(&context,0xd04e + 68+20,90);
    memcpy(memory+0x6606,"Enemy",6);memcpy(memory+0xd04e,"Hero",5);memcpy(memory+0xd04e + 68,"Partner",8);
    jx_native_battle_begin(&window,&context,1,0,0xffff);
    uint16_t hp,max;assert(jx_native_battle_hp(&window,0,&hp,&max) && hp==400 && max==500);
    assert(jx_native_battle_hp(&window,1,&hp,&max) && hp==100 && max==200);
    assert(!jx_native_battle_hp(&window,2,&hp,&max));
    jx_set_word(&context,0x6614,0xfffa);
    assert(jx_native_battle_hp(&window,0,&hp,&max) && hp==0 && max==500);
    jx_set_word(&context,0xd04e + 16,175);jx_set_word(&context,0xd04e + 20,220);
    assert(jx_native_battle_hp(&window,1,&hp,&max) && hp==175 && max==220);
    jx_native_battle_begin(&window,&context,1,0,1);
    assert(jx_native_battle_hp(&window,2,&hp,&max) && hp==70 && max==90);
    window.battle_visible=1;jx_native_window_present(&window);
    assert(GetPixel((HDC)window.back_dc,30,10)==RGB(18,23,30));
    assert(jx_native_window_post_scan(&window,0x42));assert(jx_native_window_pump(&window));
    jx_native_window_present(&window);assert(GetPixel((HDC)window.back_dc,30,10)==RGB(18,23,30));
    DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(unsigned i=0;i<40;i++){
        RECT target={0,0,(i%2)?1000:800,600};assert(AdjustWindowRect(&target,WS_OVERLAPPEDWINDOW,FALSE));
        assert(SetWindowPos((HWND)window.handle,NULL,0,0,target.right-target.left,target.bottom-target.top,SWP_NOMOVE|SWP_NOZORDER));
        jx_native_window_present(&window);assert(GetClientRect((HWND)window.handle,&client));
        dc=GetDC((HWND)window.handle);assert(dc);
        assert(GetPixel(dc,client.right/2,client.bottom/2)==RGB(252,252,252));ReleaseDC((HWND)window.handle,dc);
        assert(window.back_width==client.right && window.back_height==client.bottom);
    }
    assert(jx_word(&context,0x6614)==0xfffa && jx_word(&context,0xd04e + 16)==175);
    jx_native_battle_end(&window);assert(!jx_native_battle_hp(&window,0,&hp,&max));
    assert(GetPixel((HDC)window.back_dc,30,10)==RGB(252,252,252));
    puts("PASS: live single/dual battle HP, maximum, death clamp, F8, resize and cleanup; game memory unchanged");
    assert(window.buffered_paints==window.paints);
    assert(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdi_before+2);
    jx_native_window_close(&window);jx_hd_close(window.hd);window.hd=NULL;
    assert(!window.back_dc && !window.back_bitmap && !window.back_original);
    puts("PASS: HD pack enables 1280x800 client, 8:5 viewport and F8 comparison");
    puts("PASS: all paints compose offscreen, repeated resize stays stable and GDI objects are released");
    for(unsigned i=0;i<768;i++)palette[i]=32;
    memset(palette,0,3);palette[3]=16;palette[4]=32;palette[5]=48;
    window.hd=jx_hd_open("build/hd-fixture");assert(window.hd);window.hd_disabled=0;
    assert(jx_native_window_open(&window));
    const uint8_t source[]={1,2,255,1};memcpy(memory+0x2000,source,sizeof source);
    for(unsigned p=0;p<4;p++)video.planes[(10+p%2)%4][(12+p/2)*80+(10+p%2)/4]=source[p];
    jx_hd_blit(window.hd,&context,&video,(uint16_t[]){10,12,2,2,0,0x200},-1);
    assert(jx_native_save_browser(&window,memory,2)==0);
    assert(window.save_scene[6*160+5]==0x2255aa);
    window.hd_disabled=1;assert(jx_native_save_browser(&window,memory,2)==0);
    assert(window.save_scene[6*160+5]==0x4080c0);
    jx_native_window_close(&window);jx_hd_close(window.hd);window.hd=NULL;
    puts("PASS: save thumbnail captures HD framebuffer and follows F8 original mode");
    return 0;
}
