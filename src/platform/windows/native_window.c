#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <mmsystem.h>
#include <wchar.h>
#include "native_window.h"
/* Pick from the live game palette so native glyphs retain their pixel shape
 * in both the original and HD presentation, without recoloring scenery. */
uint16_t jx_native_selection_color(const JxNativeWindow *d) {
    if(!d->palette)return 47;
    unsigned phase=(unsigned)(GetTickCount64()%1000);
    unsigned glow=phase<500?phase:1000-phase;
    int red=200+(int)(55*glow/500),green=150+(int)(85*glow/500),blue=20+(int)(45*glow/500);
    unsigned best=0,distance=~0u;
    for(unsigned i=0;i<256;i++) {
        int r=(int)d->palette[i*3]*4-red,g=(int)d->palette[i*3+1]*4-green,b=(int)d->palette[i*3+2]*4-blue;
        unsigned delta=(unsigned)(r*r+g*g+b*b);
        if(delta<distance){distance=delta;best=i;}
    }
    return (uint16_t)best;
}
/* Keep the title artwork intact; selection is conveyed by the lettering. */
static void paint_title_menu(JxNativeWindow *d,HDC dc,int left,int top,int width,int height) {
    if(!d->title_visible || width<=0 || height<=0)return;
    int saved=SaveDC(dc);if(!saved)return;
    unsigned phase=(unsigned)(GetTickCount64()%1000);
    unsigned glow=phase<500?phase:1000-phase;
    for(unsigned i=0;i<2;i++) {
        COLORREF color=i==d->title_selection?RGB(200+55*glow/500,150+85*glow/500,20+45*glow/500):RGB(255,255,255);
        /* The original 16x16 resource glyphs, enlarged on the game's pixel grid. */
        for(int pass=1;pass>=0;pass--) {
            SetDCBrushColor(dc,pass?RGB(25,19,9):color);
            for(unsigned ch=0;ch<4;ch++)for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++) {
                if(!(d->title_glyphs[i][ch][y*2+x/8]&(0x80>>(x%8))))continue;
                int px=200+(int)ch*16+(int)x+pass,py=50+(int)i*20+(int)y+pass;
                RECT pixel={left+px*width/320,top+py*height/200,
                    left+(px+1)*width/320,top+(py+1)*height/200};
                FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
            }
        }
    }
    RestoreDC(dc,saved);
}
uint16_t jx_native_title_menu(JxNativeWindow *d,JxEventContext *c,JxFarPointer event) {
    const wchar_t *labels[]={L"新的故事",L"旧的回忆"};
    uint8_t previous_glyph[32];memcpy(previous_glyph,jx_ds(c)+0x2724,32);
    for(unsigned row=0;row<2;row++)for(unsigned ch=0;ch<4;ch++) {
        char gb[2];
        if(WideCharToMultiByte(936,0,&labels[row][ch],1,gb,2,NULL,NULL)!=2)continue;
        (void)jx_font_load_double(c,(uint16_t)(((uint8_t)gb[0]<<8)|(uint8_t)gb[1]));
        memcpy(d->title_glyphs[row][ch],jx_ds(c)+0x2724,32);
    }
    memcpy(jx_ds(c)+0x2724,previous_glyph,32);
    d->title_selection=0;d->title_visible=1;jx_native_window_present(d);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    for(;;) {
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event.offset,event.segment)!=0) {}
        uint16_t type=jx_far_word(c,event.segment,event.offset);
        uint16_t key=jx_far_word(c,event.segment,(uint16_t)(event.offset+2));
        if(type==1 && (key==0x50 || key==0x48)) {
            d->title_selection^=1;jx_native_window_present(d);
        }
        if(type==0 && (key==0x1c || key==0x39)) {
            d->title_visible=0;jx_native_window_present(d);return (uint16_t)d->title_selection;
        }
    }
}
/* Presentation-only battle status: never writes the original game memory. */
void jx_native_battle_begin(JxNativeWindow *d,JxEventContext *c,uint16_t encounter,uint16_t first,uint16_t second) {
    d->battle_context=c;d->battle_actors[0]=first;d->battle_actors[1]=second;
    d->battle_enemy_max=jx_word(c,(uint16_t)(0x499e + encounter*30+14));d->battle_visible=0;
}
void jx_native_battle_end(JxNativeWindow *d) {
    d->battle_visible=0;d->battle_context=NULL;jx_native_window_present(d);
}
int jx_native_battle_hp(JxNativeWindow *d,unsigned slot,uint16_t *current,uint16_t *maximum) {
    JxEventContext *c=d->battle_context;if(!c || slot>2)return 0;
    uint16_t value;
    if(!slot){value=jx_word(c,0x6614);*maximum=d->battle_enemy_max;}
    else {
        uint16_t actor=d->battle_actors[slot-1];
        if(actor>=16 || (slot==2 && actor==d->battle_actors[0]))return 0;
        uint16_t base=(uint16_t)(0xd04e + actor*68);
        value=jx_word(c,(uint16_t)(base+16));*maximum=jx_word(c,(uint16_t)(base+20));
    }
    *current=jx_signed_word(value)>0?value:0;return 1;
}
static void paint_battle_status(JxNativeWindow *d,HDC dc,int left,int top,int width,int height) {
    if(!d->battle_visible || !d->battle_context || width<=0 || height<=0)return;
    int saved=SaveDC(dc);if(!saved)return;
    IntersectClipRect(dc,left,top,left+width,top+height);
    int font_height=height*21/800;if(font_height<12)font_height=12;
    HFONT font=CreateFontW(-font_height,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei");
    if(font)SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);
    int partner=d->battle_actors[1]<16 && d->battle_actors[1]!=d->battle_actors[0];
    for(unsigned slot=0;slot<3;slot++) {
        uint16_t hp,max;if(!jx_native_battle_hp(d,slot,&hp,&max))continue;
        int x=left+(slot==0?12:slot==1 && partner?438:854)*width/1280;
        RECT panel={x,top+8*height/800,x+400*width/1280,top+78*height/800};
        SetDCBrushColor(dc,RGB(18,23,30));FillRect(dc,&panel,(HBRUSH)GetStockObject(DC_BRUSH));
        SetDCBrushColor(dc,slot?RGB(94,180,136):RGB(221,112,98));FrameRect(dc,&panel,(HBRUSH)GetStockObject(DC_BRUSH));
        uint16_t base=slot?(uint16_t)(0xd04e + d->battle_actors[slot-1]*68):0x6606;
        char raw[12];unsigned count=0;const uint8_t *ds=jx_ds(d->battle_context);
        while(count<sizeof raw && ds[base+count]){raw[count]=(char)ds[base+count];count++;}
        wchar_t name[16]={0},line[64];MultiByteToWideChar(936,0,raw,(int)count,name,15);
        RECT text=panel;text.left+=10*width/1280;text.top+=3*height/800;
        SetTextColor(dc,slot?RGB(151,232,185):RGB(255,170,151));
        swprintf(line,64,L"%ls  %ls",slot?L"我方":L"敌方",name);
        DrawTextW(dc,line,-1,&text,DT_SINGLELINE|DT_TOP|DT_NOPREFIX|DT_END_ELLIPSIS);
        text.top+=30*height/800;SetTextColor(dc,RGB(255,245,220));
        swprintf(line,64,L"生命  %u / %u",(unsigned)hp,(unsigned)max);
        DrawTextW(dc,line,-1,&text,DT_SINGLELINE|DT_TOP|DT_NOPREFIX|DT_END_ELLIPSIS);
    }
    RestoreDC(dc,saved);if(font)DeleteObject(font);
}
static int prepare_backbuffer(JxNativeWindow *d,HDC dc,int width,int height) {
    if(width<=0 || height<=0)return 0;
    if(!d->back_dc)d->back_dc=CreateCompatibleDC(dc);
    if(!d->back_dc)return 0;
    if(d->back_bitmap && d->back_width==width && d->back_height==height)return 1;
    HBITMAP bitmap=CreateCompatibleBitmap(dc,width,height);if(!bitmap)return 0;
    HGDIOBJ previous=SelectObject((HDC)d->back_dc,bitmap);
    if(!previous || previous==HGDI_ERROR){DeleteObject(bitmap);return 0;}
    if(d->back_bitmap)DeleteObject((HBITMAP)d->back_bitmap);else d->back_original=previous;
    d->back_bitmap=bitmap;d->back_width=width;d->back_height=height;return 1;
}
static void release_backbuffer(JxNativeWindow *d) {
    if(d->back_dc){
        if(d->back_original)SelectObject((HDC)d->back_dc,(HGDIOBJ)d->back_original);
        if(d->back_bitmap)DeleteObject((HBITMAP)d->back_bitmap);
        DeleteDC((HDC)d->back_dc);
    }
    d->back_dc=d->back_bitmap=d->back_original=NULL;d->back_width=d->back_height=0;
}
static LRESULT CALLBACK window_proc(HWND hwnd,UINT message,WPARAM w,LPARAM l) {
    JxNativeWindow *d=(JxNativeWindow *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(message==WM_NCCREATE) {
        d=((CREATESTRUCTW *)l)->lpCreateParams;
        SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)d);d->handle=hwnd;
    }
    if(!d)return DefWindowProcW(hwnd,message,w,l);
    switch(message) {
    case WM_ERASEBKGND:return 1; /* The complete offscreen frame owns the background. */
    case WM_GETMINMAXINFO: {
        if(!d->hd)break;
        MINMAXINFO *limits=(MINMAXINFO *)l;
        RECT target={0,0,1280,800};AdjustWindowRect(&target,WS_OVERLAPPEDWINDOW,FALSE);
        if(limits->ptMaxTrackSize.x<target.right-target.left)limits->ptMaxTrackSize.x=target.right-target.left;
        if(limits->ptMaxTrackSize.y<target.bottom-target.top)limits->ptMaxTrackSize.y=target.bottom-target.top;
        return 0;
    }
    case WM_KEYDOWN:case WM_SYSKEYDOWN:case WM_KEYUP:case WM_SYSKEYUP: {
        uint8_t scan=(uint8_t)((l>>16)&0x7f);
        if(d->hd && scan==0x42) {
            if((message==WM_KEYDOWN || message==WM_SYSKEYDOWN) && !(l&((LPARAM)1<<30))) {
                d->hd_disabled=!d->hd_disabled;InvalidateRect(hwnd,NULL,FALSE);
            }
            return 0;
        }
        /* One confirmation per press; ignore OS autorepeat for action keys. */
        if((message==WM_KEYDOWN || message==WM_SYSKEYDOWN) && (l&((LPARAM)1<<30)) && (scan==0x1c || scan==0x39 || scan==1))return 0;
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
        if(!prepare_backbuffer(d,dc,size.right,size.bottom)){EndPaint(hwnd,&paint);return 0;}
        HDC back=(HDC)d->back_dc;
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
        SetStretchBltMode(back,COLORONCOLOR);
        const uint32_t *pixels=d->pixels;
        if(width==320 && d->hd && !d->hd_disabled && !d->hd_suspended) {
            pixels=jx_hd_render(d->hd,d->video,d->palette);width=1280;height=800;
            info.bmiHeader.biWidth=(LONG)width;info.bmiHeader.biHeight=-(LONG)height;
        }
        int tw=size.right,th=size.bottom,left=0,top=0;
        if(d->hd) {
            int aspect_width=width==640?4:8,aspect_height=width==640?3:5;
            if((int64_t)tw*aspect_height>(int64_t)th*aspect_width)tw=th*aspect_width/aspect_height;
            else th=tw*aspect_height/aspect_width;
            left=(size.right-tw)/2;top=(size.bottom-th)/2;
        }
        FillRect(back,&size,(HBRUSH)GetStockObject(BLACK_BRUSH));
        if(tw>0 && th>0)StretchDIBits(back,left,top,tw,th,0,0,(int)width,(int)height,pixels,&info,DIB_RGB_COLORS,SRCCOPY);
        if(!d->mode || *d->mode!=18)paint_battle_status(d,back,left,top,tw,th);
        if(!d->mode || *d->mode!=18)paint_title_menu(d,back,left,top,tw,th);
        if(BitBlt(dc,0,0,size.right,size.bottom,back,0,0,SRCCOPY))d->buffered_paints++;
        EndPaint(hwnd,&paint);return 0;
    }
    case WM_CLOSE:jx_native_window_close(d);return 0;
    default:return DefWindowProcW(hwnd,message,w,l);
    }
    return DefWindowProcW(hwnd,message,w,l);
}
int jx_native_window_open(JxNativeWindow *d) {
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={0};cls.lpfnWndProc=window_proc;cls.hInstance=instance;
    cls.hCursor=LoadCursorW(NULL,IDC_ARROW);cls.lpszClassName=L"JxNativeEngineWindow";
    if(!RegisterClassW(&cls) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return 0;
    RECT rectangle={0,0,960,600};AdjustWindowRect(&rectangle,WS_OVERLAPPEDWINDOW,FALSE);
    if(d->hd){rectangle=(RECT){0,0,1280,800};AdjustWindowRect(&rectangle,WS_OVERLAPPEDWINDOW,FALSE);}
    HWND hwnd=CreateWindowExW(0,cls.lpszClassName,L"剑侠情缘 · 原生引擎",WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,rectangle.right-rectangle.left,rectangle.bottom-rectangle.top,
        NULL,NULL,instance,d);
    if(!hwnd)return 0;
    if(d->hd)SetWindowPos(hwnd,NULL,0,0,rectangle.right-rectangle.left,rectangle.bottom-rectangle.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
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
    release_backbuffer(d);
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
