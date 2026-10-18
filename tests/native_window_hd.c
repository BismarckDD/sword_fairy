#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "native_window.h"
static JxNativeWindow window;
static JxVideoModel video;
static JxLegacyVgaModel legacy;
static uint8_t palette[768];
static uint16_t mode;
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
    DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(unsigned i=0;i<40;i++){
        RECT target={0,0,(i%2)?1000:800,600};assert(AdjustWindowRect(&target,WS_OVERLAPPEDWINDOW,FALSE));
        assert(SetWindowPos((HWND)window.handle,NULL,0,0,target.right-target.left,target.bottom-target.top,SWP_NOMOVE|SWP_NOZORDER));
        jx_native_window_present(&window);assert(GetClientRect((HWND)window.handle,&client));
        dc=GetDC((HWND)window.handle);assert(dc);
        assert(GetPixel(dc,client.right/2,client.bottom/2)==RGB(252,252,252));ReleaseDC((HWND)window.handle,dc);
        assert(window.back_width==client.right && window.back_height==client.bottom);
    }
    assert(window.buffered_paints==window.paints);
    assert(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdi_before+2);
    jx_native_window_close(&window);jx_hd_close(window.hd);window.hd=NULL;
    assert(!window.back_dc && !window.back_bitmap && !window.back_original);
    puts("PASS: HD pack enables 1280x800 client, 8:5 viewport and F8 comparison");
    puts("PASS: all paints compose offscreen, repeated resize stays stable and GDI objects are released");
    return 0;
}
