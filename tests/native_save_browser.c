/* Isolated file and actual Win32 browser regression tests. Run from a fresh
 * test directory: the test creates its own userdata, never real player saves. */
#include <assert.h>
#include "../src/platform/windows/native_saves.c"
#include "save_format.h"
static JxNativeWindow window;
static uint8_t ds[65536],restored[65536];
void jx_native_window_present(JxNativeWindow *w){(void)w;}
void jx_native_window_sync_ticks(JxNativeWindow *w){(void)w;}
static unsigned automation_mode;
static void screenshot(HWND hwnd) {
    RECT r;GetClientRect(hwnd,&r);int width=r.right,height=r.bottom;
    HDC dc=GetDC(hwnd),mem=CreateCompatibleDC(dc);BITMAPINFO info={0};info.bmiHeader.biSize=sizeof info.bmiHeader;info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    void *pixels;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);HGDIOBJ old=SelectObject(mem,bitmap);PrintWindow(hwnd,mem,PW_CLIENTONLY);
    BITMAPFILEHEADER header={0};header.bfType=0x4d42;header.bfOffBits=sizeof header+sizeof info.bmiHeader;header.bfSize=header.bfOffBits+width*height*4;
    FILE *f=fopen("browser.bmp","wb");assert(f);assert(fwrite(&header,1,sizeof header,f)==sizeof header);assert(fwrite(&info.bmiHeader,1,sizeof info.bmiHeader,f)==sizeof info.bmiHeader);assert(fwrite(pixels,4,width*height,f)==(size_t)(width*height));fclose(f);
    SelectObject(mem,old);DeleteObject(bitmap);DeleteDC(mem);ReleaseDC(hwnd,dc);
}
static void CALLBACK automate(HWND hwnd,UINT msg,UINT_PTR id,DWORD tick) {
    (void)hwnd;(void)msg;(void)tick;HWND browser=FindWindowW(L"JxSaveBrowser",NULL);if(!browser)return;KillTimer(NULL,id);
    if(!automation_mode){screenshot(browser);for(int i=0;i<9;i++)PostMessageW(browser,WM_KEYDOWN,VK_RIGHT,0);PostMessageW(browser,WM_KEYDOWN,VK_UP,0);}
    PostMessageW(browser,WM_KEYDOWN,VK_RETURN,0);
}
int main(void) {
    assert(GetFileAttributesA("main_decompiled")==INVALID_FILE_ATTRIBUTES);
    assert(_mkdir("main_decompiled")==0);assert(_mkdir("main_decompiled/userdata")==0);
    for(unsigned i=0;i<sizeof ds;i++)ds[i]=(uint8_t)(i*37);ds[0x9334]=5;ds[0x9335]=0;
    window.handle=CreateWindowW(L"STATIC",L"Test",WS_OVERLAPPEDWINDOW,0,0,960,600,NULL,NULL,GetModuleHandleW(NULL),NULL);assert(window.handle);window.save_scene_valid=1;
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
    assert(SetTimer(NULL,0,250,automate));assert(jx_native_save_browser(&window,ds,0)==98);
    automation_mode=1;last_slot=20;assert(SetTimer(NULL,0,250,automate));assert(jx_native_save_browser(&window,ds,1)==20);inspect(&b,20);assert(b.slots[20].state==1 && b.slots[20].preview);
    b.selected=98;page_move(&b,1);assert(b.selected==8);page_move(&b,-1);assert(b.selected==98);
    DestroyWindow((HWND)window.handle);puts("PASS: slots 01/10/11/99; original-format restoration; empty/corrupt/legacy saves; preview matching; real browser paging and save.");return 0;
}
