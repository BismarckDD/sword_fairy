#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "native_window.h"

static JxNativeWindow window;
static JxVideoModel video;
static uint8_t palette[768];
static unsigned callbacks,confirmations;
static void tick(void *opaque){(void)opaque;++callbacks;}
static void scan(void *opaque,uint8_t value){(void)opaque;if(value==0x1c || value==0x39 || value==1)++confirmations;}

int main(void){
    window.video=&video;window.palette=palette;window.tick=tick;window.scan=scan;
    assert(jx_native_window_open(&window));
    ShowWindow((HWND)window.handle,SW_HIDE);
    /* A delayed/coalesced message must not discard elapsed DOS ticks. */
    Sleep(330);
    assert(jx_native_window_pump(&window));
    assert(callbacks>=6);
    uint64_t due=(GetTickCount64()-window.tick_origin)*1193182/65536000;
    assert(window.ticks<=due && due-window.ticks<=1);
    /* Duplicate OS notifications must not invent extra game time. */
    for(unsigned i=0;i<50;i++)PostMessageW((HWND)window.handle,WM_TIMER,1,0);
    assert(jx_native_window_pump(&window));
    due=(GetTickCount64()-window.tick_origin)*1193182/65536000;
    assert(window.ticks<=due && due-window.ticks<=1);
    while(window.ticks<100)assert(jx_native_window_pump(&window));
    uint64_t elapsed=GetTickCount64()-window.tick_origin;
    assert(elapsed>=5492 && elapsed<5700);
    assert(callbacks==window.ticks);
    printf("PASS: 100 BIOS ticks in %llu ms; stalled and duplicate messages handled\n",
        (unsigned long long)elapsed);
    assert(jx_native_window_wait_retrace(&window));
    uint64_t begin=GetTickCount64();
    for(unsigned i=0;i<70;i++)assert(jx_native_window_wait_retrace(&window));
    elapsed=GetTickCount64()-begin;
    assert(elapsed>=980 && elapsed<1150);
    printf("PASS: 70 palette retraces in %llu ms\n",(unsigned long long)elapsed);
    unsigned before=callbacks;
    Sleep(330); /* Audio loading must not become subtitle catch-up ticks. */
    jx_native_window_sync_ticks(&window);
    assert(jx_native_window_pump(&window));
    assert(callbacks==before);
    Sleep(60);
    assert(jx_native_window_pump(&window));
    assert(callbacks==before+1);
    puts("PASS: opening audio preparation excluded from subtitle clock");
    const unsigned keys[]={0x1c,0x39,1};
    for(unsigned i=0;i<3;i++){
        LPARAM detail=(LPARAM)keys[i]<<16;
        assert(PostMessageW((HWND)window.handle,WM_KEYDOWN,0,detail));
        assert(PostMessageW((HWND)window.handle,WM_KEYDOWN,0,detail|((LPARAM)1<<30)));
        assert(PostMessageW((HWND)window.handle,WM_KEYUP,0,detail|((LPARAM)1<<31)));
    }
    assert(jx_native_window_pump(&window));assert(confirmations==3);
    puts("PASS: Enter/Space/Escape autorepeat cannot cause multiple confirmations");
    assert(jx_native_window_post_close(&window));
    assert(!jx_native_window_pump(&window));
    return 0;
}
