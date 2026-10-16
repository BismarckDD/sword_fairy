#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <direct.h>
#include "native_window.h"
#define SAVE_SIZE 17266
#define SLOTS 99
#define PREVIEW_PIXELS (160*100)
typedef struct {uint32_t magic,hash; FILETIME time; uint32_t pixels[PREVIEW_PIXELS];} Preview;
typedef struct {int state,scene,preview; FILETIME time; uint32_t hash;} Slot;
typedef struct {
    JxNativeWindow *window; uint8_t *ds; HWND hwnd,list,detail;
    Slot slots[SLOTS]; Preview image; int selected,writing,done,result;
} Browser;
static int last_slot;
/* Names confirmed by the existing walkthrough map transitions. */
static void scene_text(int scene,wchar_t *text,size_t size) {
    const wchar_t *name=scene==5?L"临安城":scene==11?L"酒楼一楼":scene==14?L"酒楼二楼":NULL;
    if(name)swprintf(text,size,L"%ls",name);else swprintf(text,size,L"场景 %03d",scene);
}
static uint32_t hash_bytes(const uint8_t *bytes,size_t count) {
    uint32_t hash=2166136261u;for(size_t i=0;i<count;i++)hash=(hash^bytes[i])*16777619u;return hash;
}
static void path_for(char *path,size_t size,int slot,const char *suffix) {
    snprintf(path,size,"main_decompiled/userdata/SAVE%d.%s",slot,suffix);
}
static FILE *read_slot(int slot,char *path,size_t size) {
    path_for(path,size,slot,"DAT");FILE *f=fopen(path,"rb");
    if(!f && GetFileAttributesA(path)==INVALID_FILE_ATTRIBUTES){snprintf(path,size,"SAVE%d.DAT",slot);f=fopen(path,"rb");}
    return f;
}
static void inspect(Browser *b,int slot) {
    char path[160];uint8_t bytes[SAVE_SIZE];Slot *s=&b->slots[slot];memset(s,0,sizeof *s);
    FILE *f=read_slot(slot,path,sizeof path);
    if(!f){s->state=GetFileAttributesA(path)==INVALID_FILE_ATTRIBUTES?0:2;return;}
    s->state=2;
    if(fread(bytes,1,sizeof bytes,f)==sizeof bytes && fgetc(f)==EOF && !ferror(f)) {
        s->scene=bytes[1404]|((unsigned)bytes[1405]<<8);s->hash=hash_bytes(bytes,sizeof bytes);
        if(s->scene<220)s->state=1;
    }
    fclose(f);WIN32_FILE_ATTRIBUTE_DATA info;
    if(GetFileAttributesExA(path,GetFileExInfoStandard,&info))s->time=info.ftLastWriteTime;
    path_for(path,sizeof path,slot,"preview");f=fopen(path,"rb");
    if(f){Preview p;if(fread(&p,1,sizeof p,f)==sizeof p && p.magic==0x3153584a && p.hash==s->hash && s->state==1){s->preview=1;s->time=p.time;}fclose(f);}
}
static void time_text(FILETIME time,wchar_t *text,size_t size) {
    FILETIME local;SYSTEMTIME st;
    if(!FileTimeToLocalFileTime(&time,&local)||!FileTimeToSystemTime(&local,&st)){swprintf(text,size,L"时间未知");return;}
    swprintf(text,size,L"%04u-%02u-%02u %02u:%02u:%02u",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
}
static void refresh(Browser *b) {
    SendMessageW(b->list,LB_RESETCONTENT,0,0);int page=b->selected/10;
    for(int i=page*10;i<SLOTS && i<page*10+10;i++) {
        wchar_t line[160],time[40],place[40];Slot *s=&b->slots[i];time_text(s->time,time,40);
        if(!s->state)swprintf(line,160,L"%02d    空存档",i+1);
        else if(s->state==2)swprintf(line,160,L"%02d    存档异常",i+1);
        else {scene_text(s->scene,place,40);swprintf(line,160,L"%02d    %ls    %ls",i+1,place,time);}
        SendMessageW(b->list,LB_ADDSTRING,0,(LPARAM)line);
    }
    SendMessageW(b->list,LB_SETCURSEL,b->selected%10,0);
    Slot *s=&b->slots[b->selected];wchar_t caption[120],text[200],time[40],place[40];
    swprintf(caption,120,L"%ls · 第 %d / 10 页 · 存档 %02d",b->writing?L"保存游戏":L"读取存档",page+1,b->selected+1);SetWindowTextW(b->hwnd,caption);
    time_text(s->time,time,40);
    if(s->state==1){scene_text(s->scene,place,40);swprintf(text,200,L"%ls\n%ls\n%ls",place,time,s->preview?L"保存时间":L"文件修改时间 · 暂无预览");}
    else swprintf(text,200,L"%ls",s->state?L"存档异常，无法读取":L"空存档");
    SetWindowTextW(b->detail,text);memset(&b->image,0,sizeof b->image);
    if(s->preview){char path[160];path_for(path,sizeof path,b->selected,"preview");FILE *f=fopen(path,"rb");if(f){if(fread(&b->image,1,sizeof b->image,f)!=sizeof b->image)b->image.magic=0;fclose(f);}}
    InvalidateRect(b->hwnd,NULL,TRUE);
}
static int commit_save(Browser *b) {
    static const unsigned blocks[][2]={{0x8894,2},{0x889a,2},{0xd04e,1088},{0x8752,312},{0x9334,2},{0x3466,220},{0x665a,6600},{0xb686,6600},{0xacec,2200}};
    uint8_t bytes[SAVE_SIZE];size_t at=0;
    for(unsigned i=0;i<sizeof blocks/sizeof blocks[0];i++){memcpy(bytes+at,b->ds+blocks[i][0],blocks[i][1]);at+=blocks[i][1];}
    for(unsigned i=0;i<30;i++){unsigned npc=0x889c+86*i;memcpy(bytes+at,b->ds+npc+64,6);at+=6;memcpy(bytes+at,b->ds+npc+60,2);at+=2;}
    (void)_mkdir("main_decompiled/userdata");char path[160],temp[160];path_for(path,sizeof path,b->selected,"DAT");path_for(temp,sizeof temp,b->selected,"DAT.tmp");
    FILE *f=fopen(temp,"wb");if(!f)return 0;int ok=fwrite(bytes,1,sizeof bytes,f)==sizeof bytes;if(fflush(f))ok=0;if(fclose(f))ok=0;
    if(!ok || !MoveFileExA(temp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){DeleteFileA(temp);return 0;}
    Preview p={0};p.magic=0x3153584a;p.hash=hash_bytes(bytes,sizeof bytes);GetSystemTimeAsFileTime(&p.time);memcpy(p.pixels,b->window->save_scene,sizeof p.pixels);
    path_for(path,sizeof path,b->selected,"preview");path_for(temp,sizeof temp,b->selected,"preview.tmp");f=fopen(temp,"wb");ok=f!=NULL;
    if(f){if(fwrite(&p,1,sizeof p,f)!=sizeof p)ok=0;if(fflush(f))ok=0;if(fclose(f))ok=0;}
    if(!ok || !MoveFileExA(temp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(temp);MessageBoxW(b->hwnd,L"游戏进度已保存，但预览写入失败。",L"保存提示",MB_OK|MB_ICONWARNING);
    }
    return 1;
}
static void confirm(Browser *b) {
    inspect(b,b->selected);Slot *s=&b->slots[b->selected];
    if(!b->writing && s->state!=1){MessageBoxW(b->hwnd,s->state?L"存档异常，无法读取。":L"这是空存档。",L"读取存档",MB_OK);refresh(b);return;}
    if(b->writing) {
        wchar_t text[100];swprintf(text,100,L"是否覆盖存档 %02d？",b->selected+1);
        if(s->state && MessageBoxW(b->hwnd,text,L"确认覆盖",MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)!=IDYES)return;
        if(!commit_save(b)){MessageBoxW(b->hwnd,L"保存失败，原存档已保留。请检查磁盘空间和目录权限。",L"保存失败",MB_OK|MB_ICONERROR);return;}
    }
    last_slot=b->selected;b->result=b->selected;b->done=1;
}
static void page_move(Browser *b,int delta) {
    int page=(b->selected/10+delta+10)%10,row=b->selected%10;b->selected=page*10+row;if(b->selected>=SLOTS)b->selected=SLOTS-1;refresh(b);
}
static LRESULT CALLBACK browser_proc(HWND hwnd,UINT msg,WPARAM w,LPARAM l) {
    Browser *b=(Browser *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(msg==WM_NCCREATE){b=((CREATESTRUCTW *)l)->lpCreateParams;SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)b);b->hwnd=hwnd;}
    if(!b)return DefWindowProcW(hwnd,msg,w,l);
    if(msg==WM_CLOSE){b->done=1;return 0;}
    if(msg==WM_COMMAND){int id=LOWORD(w);
        if(id==10 && HIWORD(w)==LBN_SELCHANGE){int row=(int)SendMessageW(b->list,LB_GETCURSEL,0,0);if(row>=0){b->selected=b->selected/10*10+row;refresh(b);}return 0;}
        if(id==10 && HIWORD(w)==LBN_DBLCLK){confirm(b);return 0;}
        if(id==1)confirm(b);if(id==2)b->done=1;if(id==3)page_move(b,-1);if(id==4)page_move(b,1);return 0;
    }
    if(msg==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);RECT r={470,35,790,235};FillRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));
        if(b->image.magic==0x3153584a){BITMAPINFO info={0};info.bmiHeader.biSize=sizeof info.bmiHeader;info.bmiHeader.biWidth=160;info.bmiHeader.biHeight=-100;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;StretchDIBits(dc,470,35,320,200,0,0,160,100,b->image.pixels,&info,DIB_RGB_COLORS,SRCCOPY);}
        else {SetTextColor(dc,RGB(220,220,220));SetBkMode(dc,TRANSPARENT);DrawTextW(dc,L"暂无预览",-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);}
        EndPaint(hwnd,&ps);return 0;}
    return DefWindowProcW(hwnd,msg,w,l);
}
int jx_native_save_browser(JxNativeWindow *window,uint8_t *ds,int writing) {
    if(writing==2 || !window->save_scene_valid){
        jx_native_window_present(window);unsigned width=window->mode && *window->mode==18?640:320,height=width==640?480:200;
        for(unsigned y=0;y<100;y++)for(unsigned x=0;x<160;x++)window->save_scene[y*160+x]=window->pixels[(y*height/100)*width+x*width/160];
        window->save_scene_valid=1;if(writing==2)return 0;
    }
    Browser b={0};b.window=window;b.ds=ds;b.writing=writing;b.selected=last_slot;b.result=-1;
    for(int i=0;i<SLOTS;i++)inspect(&b,i);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW cls={0};cls.lpfnWndProc=browser_proc;cls.hInstance=instance;cls.hCursor=LoadCursorW(NULL,IDC_ARROW);cls.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);cls.lpszClassName=L"JxSaveBrowser";
    if(!RegisterClassW(&cls) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return -1;
    RECT parent;GetWindowRect((HWND)window->handle,&parent);HWND hwnd=CreateWindowExW(WS_EX_DLGMODALFRAME,cls.lpszClassName,L"存档",WS_CAPTION|WS_SYSMENU|WS_POPUP,parent.left+30,parent.top+50,840,460,(HWND)window->handle,NULL,instance,&b);if(!hwnd)return -1;
    HFONT font=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Microsoft YaHei");
    b.list=CreateWindowW(L"LISTBOX",NULL,WS_CHILD|WS_VISIBLE|WS_BORDER|LBS_NOTIFY,20,25,425,310,hwnd,(HMENU)10,instance,NULL);SendMessageW(b.list,WM_SETFONT,(WPARAM)font,TRUE);SendMessageW(b.list,LB_SETITEMHEIGHT,0,28);
    b.detail=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,470,250,340,85,hwnd,NULL,instance,NULL);
    const wchar_t *labels[]={L"确认",L"返回",L"上一页",L"下一页"};for(int i=0;i<4;i++){HWND child=CreateWindowW(L"BUTTON",labels[i],WS_CHILD|WS_VISIBLE|WS_TABSTOP,20+i*115,355,100,32,hwnd,(HMENU)(INT_PTR)(i+1),instance,NULL);SendMessageW(child,WM_SETFONT,(WPARAM)font,TRUE);}
    HWND hint=CreateWindowW(L"STATIC",L"↑↓ 选择   ←→ 翻页   Enter 确认   Esc 返回",WS_CHILD|WS_VISIBLE,20,395,760,24,hwnd,NULL,instance,NULL);SendMessageW(hint,WM_SETFONT,(WPARAM)font,TRUE);SendMessageW(b.detail,WM_SETFONT,(WPARAM)font,TRUE);
    refresh(&b);EnableWindow((HWND)window->handle,FALSE);ShowWindow(hwnd,SW_SHOW);SetFocus(b.list);
    MSG message;int quit=0;
    while(!b.done){int got=GetMessageW(&message,NULL,0,0);if(got<=0){quit=got==0;break;}
        if(message.message==WM_KEYDOWN && (message.hwnd==hwnd || IsChild(hwnd,message.hwnd))){
            switch(message.wParam){case VK_ESCAPE:b.done=1;continue;case VK_RETURN:confirm(&b);continue;case VK_LEFT:case VK_PRIOR:page_move(&b,-1);continue;case VK_RIGHT:case VK_NEXT:page_move(&b,1);continue;
            case VK_UP:case VK_DOWN:{int first=b.selected/10*10,count=first==90?9:10; b.selected=first+(b.selected-first+(message.wParam==VK_UP?count-1:1))%count;refresh(&b);continue;}}
        }
        TranslateMessage(&message);DispatchMessageW(&message);
    }
    DestroyWindow(hwnd);DeleteObject(font);EnableWindow((HWND)window->handle,TRUE);SetForegroundWindow((HWND)window->handle);jx_native_window_sync_ticks(window);if(quit)PostQuitMessage((int)message.wParam);return b.result;
}
