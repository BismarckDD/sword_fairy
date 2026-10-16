#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <mmsystem.h>
#include "native_window.h"
static LRESULT CALLBACK window_proc(HWND hwnd,UINT message,WPARAM w,LPARAM l) {
    JxNativeWindow *d=(JxNativeWindow *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(message==WM_NCCREATE) {
        d=((CREATESTRUCTW *)l)->lpCreateParams;
        SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)d);d->handle=hwnd;
    }
    if(!d)return DefWindowProcW(hwnd,message,w,l);
    switch(message) {
    case WM_KEYDOWN:case WM_SYSKEYDOWN:case WM_KEYUP:case WM_SYSKEYUP: {
        uint8_t scan=(uint8_t)((l>>16)&0x7f);
        d->pressed[scan]=(uint8_t)(message==WM_KEYDOWN || message==WM_SYSKEYDOWN);
        if(message==WM_KEYUP || message==WM_SYSKEYUP)scan|=0x80;
        d->scan(d->opaque,scan);return 0;
    }
    case WM_KILLFOCUS:
        for(unsigned scan=1;scan<128;scan++)if(d->pressed[scan]){
            d->pressed[scan]=0;d->scan(d->opaque,(uint8_t)(scan|0x80));
        }
        return 0;
    case WM_TIMER:InvalidateRect(hwnd,NULL,FALSE);return 0;
    case WM_PAINT: {
        d->paints++;
        PAINTSTRUCT paint;HDC dc=BeginPaint(hwnd,&paint);RECT size;GetClientRect(hwnd,&size);
        unsigned start=(unsigned)d->video->start_high<<8;
        unsigned width=d->mode && *d->mode==18?640:320,height=width==640?480:200;
        if(width==640)++d->planar_paints;
        for(unsigned y=0;y<height;y++)for(unsigned x=0;x<width;x++) {
            unsigned index=width==640?jx_legacy_vga_model_pixel(d->legacy,(uint16_t)x,(uint16_t)y):d->video->planes[x%4][(start+y*80+x/4)&65535];
            const uint8_t *rgb=d->palette+index*3;
            d->pixels[y*width+x]=((uint32_t)(rgb[0]*4)<<16)|((uint32_t)(rgb[1]*4)<<8)|(rgb[2]*4);
        }
        BITMAPINFO info={0};info.bmiHeader.biSize=sizeof info.bmiHeader;
        info.bmiHeader.biWidth=(LONG)width;info.bmiHeader.biHeight=-(LONG)height;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        SetStretchBltMode(dc,COLORONCOLOR);
        StretchDIBits(dc,0,0,size.right,size.bottom,0,0,(int)width,(int)height,d->pixels,&info,DIB_RGB_COLORS,SRCCOPY);
        EndPaint(hwnd,&paint);return 0;
    }
    case WM_CLOSE:jx_native_window_close(d);return 0;
    default:return DefWindowProcW(hwnd,message,w,l);
    }
}
int jx_native_window_open(JxNativeWindow *d) {
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={0};cls.lpfnWndProc=window_proc;cls.hInstance=instance;
    cls.hCursor=LoadCursorW(NULL,IDC_ARROW);cls.lpszClassName=L"JxNativeEngineWindow";
    if(!RegisterClassW(&cls) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return 0;
    RECT rectangle={0,0,960,600};AdjustWindowRect(&rectangle,WS_OVERLAPPEDWINDOW,FALSE);
    HWND hwnd=CreateWindowExW(0,cls.lpszClassName,L"剑侠情缘 · 原生引擎",WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,rectangle.right-rectangle.left,rectangle.bottom-rectangle.top,
        NULL,NULL,instance,d);
    if(!hwnd)return 0;
    d->closed=0;d->ticks=0;d->retrace_frame=0;d->tick_origin=GetTickCount64();
    LARGE_INTEGER frequency,counter;
    QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&counter);
    d->retrace_frequency=(uint64_t)frequency.QuadPart;d->retrace_origin=(uint64_t)counter.QuadPart;
    if(!SetTimer(hwnd,1,55,NULL)){DestroyWindow(hwnd);d->handle=NULL;return 0;}
    d->timer_resolution=timeBeginPeriod(1)==TIMERR_NOERROR;
    ShowWindow(hwnd,SW_SHOW);UpdateWindow(hwnd);return 1;
}
void jx_native_window_sync_ticks(JxNativeWindow *d) {
    /* Discard time spent preparing the opening audio, preserving ticks
     * already delivered. Do not alter the independent VGA refresh clock. */
    d->tick_origin=GetTickCount64()-(uint64_t)d->ticks*65536000/1193182;
}
int jx_native_window_pump(JxNativeWindow *d) {
    MSG message;
    while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    if(d->closed)return 0;
    /* WM_TIMER rounds intervals and coalesces missed messages. Counting one
     * DOS tick per message slows subtitles relative to independently playing
     * audio. Derive BIOS ticks from elapsed time (PIT / 65536) instead. */
    uint64_t due=(GetTickCount64()-d->tick_origin)*1193182/65536000;
    while((uint64_t)d->ticks<due){d->ticks++;d->tick(d->opaque);}
    Sleep(1);return 1;
}
void jx_native_window_present(JxNativeWindow *d) {
    if(d->handle){InvalidateRect((HWND)d->handle,NULL,FALSE);UpdateWindow((HWND)d->handle);}
}
int jx_native_window_wait_retrace(JxNativeWindow *d) {
    /* Mode 13h VGA refreshes at approximately 70 Hz. Palette fades wait on
     * this device boundary, independently of the 18.2 Hz BIOS timer. */
    LARGE_INTEGER counter;QueryPerformanceCounter(&counter);
    uint64_t frame=((uint64_t)counter.QuadPart-d->retrace_origin)*70/d->retrace_frequency;
    while(frame<=d->retrace_frame){
        if(!jx_native_window_pump(d))return 0;
        QueryPerformanceCounter(&counter);
        frame=((uint64_t)counter.QuadPart-d->retrace_origin)*70/d->retrace_frequency;
    }
    d->retrace_frame=frame;
    return 1;
}
void jx_native_window_close(JxNativeWindow *d) {
    if(d->handle)DestroyWindow((HWND)d->handle);d->handle=NULL;d->closed=1;
    if(d->timer_resolution){timeEndPeriod(1);d->timer_resolution=0;}
}
/* Bounded verification uses the same Win32 message path as keyboard input. */
int jx_native_window_post_scan(JxNativeWindow *d,uint8_t scan) {
    LPARAM detail=(LPARAM)(scan&0x7f)<<16;
    if(scan&0x80)detail|=(LPARAM)UINT32_C(0xc0000000);
    return d->handle && PostMessageW((HWND)d->handle,
        scan&0x80?WM_KEYUP:WM_KEYDOWN,0,detail);
}
int jx_native_window_post_focus_loss(JxNativeWindow *d) {
    return d->handle && PostMessageW((HWND)d->handle,WM_KILLFOCUS,0,0);
}
int jx_native_window_post_close(JxNativeWindow *d) {
    return d->handle && PostMessageW((HWND)d->handle,WM_CLOSE,0,0);
}
