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
    JxNativeWindow *window; uint8_t *ds; JxEventContext *c;
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
        DeleteFileA(temp);return 2;
    }
    return 1;
}
/* Game-native browser: the original font renderer and palette, on the
 * existing VGA page. No OS dialogs, fonts, controls or secondary window. */
static void game_text(Browser *b,unsigned x,unsigned y,const wchar_t *text,int selected) {
    char encoded[96];int length=WideCharToMultiByte(936,0,text,-1,encoded,sizeof encoded,NULL,NULL);
    if(!length)return;
    JxFarPointer scratch={0x200,0x9d00};for(int i=0;i<length;i++)jx_far_set_byte(b->c,scratch,(uint16_t)i,(uint8_t)encoded[i]);
    (void)JX_CALL(b->c,0x1f2a,0x06f2,selected?47:80,0,77);
    (void)JX_CALL(b->c,0x1f2a,0x0728,x,y,scratch.offset,scratch.segment);
}
static unsigned screen_start(Browser *b){return (unsigned)b->window->video->start_high<<8;}
static void put_pixel(Browser *b,unsigned x,unsigned y,uint8_t color) {
    b->window->video->planes[x&3][(screen_start(b)+y*80+x/4)&65535]=color;
}
static void backdrop(Browser *b,const uint8_t *pixels,int dim) {
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++) {
        unsigned color=pixels[y*320+x];put_pixel(b,x,y,dim?b->ds[0xb584+color]:(uint8_t)color);
    }
}
static void border(Browser *b,unsigned x,unsigned y,unsigned width,unsigned height) {
    for(unsigned i=0;i<width;i++){put_pixel(b,x+i,y,80);put_pixel(b,x+i,y+height-1,80);}
    for(unsigned i=0;i<height;i++){put_pixel(b,x,y+i,80);put_pixel(b,x+width-1,y+i,80);}
}
static uint8_t nearest_color(Browser *b,uint32_t color,uint16_t *cache) {
    unsigned r=(color>>16)&255,g=(color>>8)&255,blue=color&255,key=((r>>3)<<10)|((g>>3)<<5)|(blue>>3);
    if(cache[key]!=0xffff)return (uint8_t)cache[key];
    unsigned best=0,distance=~0u;
    for(unsigned i=0;i<256;i++) {
        int dr=(int)r-b->window->palette[i*3]*4,dg=(int)g-b->window->palette[i*3+1]*4,db=(int)blue-b->window->palette[i*3+2]*4;
        unsigned d=(unsigned)(dr*dr+dg*dg+db*db);if(d<distance){distance=d;best=i;}
    }
    cache[key]=(uint16_t)best;return (uint8_t)best;
}
static void draw_browser(Browser *b,const uint8_t *background) {
    backdrop(b,background,1);border(b,0,0,320,200);border(b,132,18,184,162);
    wchar_t text[96];swprintf(text,96,L"%ls  %d/10",b->writing?L"存档":L"读档",b->selected/10+1);game_text(b,4,1,text,0);
    int first=b->selected/10*10;
    for(int i=first;i<SLOTS && i<first+10;i++) {
        Slot *s=&b->slots[i];wchar_t place[40];scene_text(s->scene,place,40);
        swprintf(text,96,L"%02d %ls",i+1,!s->state?L"空存档":s->state==2?L"异常":place);
        game_text(b,4,(unsigned)(18+(i-first)*16),text,i==b->selected);
    }
    Slot *s=&b->slots[b->selected];
    if(s->state==1)game_text(b,144,1,s->preview?L"保存时间":L"文件时间",0);
    memset(&b->image,0,sizeof b->image);
    if(s->preview) {
        char path[160];path_for(path,sizeof path,b->selected,"preview");FILE *f=fopen(path,"rb");
        if(f){if(fread(&b->image,1,sizeof b->image,f)!=sizeof b->image)b->image.magic=0;fclose(f);}
    }
    if(b->image.magic==0x3153584a && b->image.hash==s->hash) {
        uint16_t cache[32768];memset(cache,255,sizeof cache);
        for(unsigned y=0;y<100;y++)for(unsigned x=0;x<160;x++)put_pixel(b,144+x,24+y,nearest_color(b,b->image.pixels[y*160+x],cache));
    } else game_text(b,152,64,L"暂无预览",0);
    if(s->state==1) {
        scene_text(s->scene,text,96);game_text(b,144,126,text,0);time_text(s->time,text,96);
        text[10]=0;game_text(b,144,143,text,0);time_text(s->time,text,96);game_text(b,144,160,text+11,0);
    } else game_text(b,144,143,s->state?L"存档异常":L"空存档",0);
    game_text(b,4,182,L"左右翻页 回车/空格确认",0);jx_native_window_present(b->window);
}
static void notice(Browser *b,const wchar_t *text) {
    (void)JX_CALL(b->c,0x1832,0x1884,132,124,184,55,0);game_text(b,136,128,text,0);
    game_text(b,136,150,L"按键返回",0);jx_native_window_present(b->window);(void)JX_CALL0(b->c,0x1f2a,0x731a);
}
static int overwrite(Browser *b) {
    const char *labels[]={"\xc8\xb7\xc8\xcf\xb8\xb2\xb8\xc7","\xc8\xa1\xcf\xfb"};
    JxFarPointer text={0x300,0x9d00},table={0x340,0x9d00};
    for(unsigned i=0;i<2;i++) {
        for(unsigned k=0;k<=strlen(labels[i]);k++)jx_far_set_byte(b->c,text,(uint16_t)(16*i+k),(uint8_t)labels[i][k]);
        uint16_t offset=(uint16_t)(text.offset+16*i);
        jx_far_set_byte(b->c,table,(uint16_t)(4*i),(uint8_t)offset);jx_far_set_byte(b->c,table,(uint16_t)(4*i+1),(uint8_t)(offset>>8));
        jx_far_set_byte(b->c,table,(uint16_t)(4*i+2),0);jx_far_set_byte(b->c,table,(uint16_t)(4*i+3),0x9d);
    }
    uint16_t selected=1;return jx_menu_select(b->c,144,40,2,table,(JxFarPointer){0x360,0x9d00},&selected)==0;
}
int jx_native_save_browser(JxNativeWindow *window,uint8_t *ds,int mode) {
    (void)ds;if(mode!=2)return -2;
    jx_native_window_present(window);unsigned width=window->mode && *window->mode==18?640:320,height=width==640?480:200;
    for(unsigned y=0;y<100;y++)for(unsigned x=0;x<160;x++)window->save_scene[y*160+x]=window->pixels[(y*height/100)*width+x*width/160];
    window->save_scene_valid=1;return 0;
}
int jx_native_game_save_browser(JxNativeWindow *window,JxEventContext *c,int writing) {
    uint8_t *ds=jx_ds(c);
    if(writing==2)return jx_native_save_browser(window,ds,2);
    if(!window->save_scene_valid)(void)jx_native_save_browser(window,ds,2);
    Browser b={0};b.window=window;b.ds=ds;b.c=c;b.selected=last_slot;b.writing=writing;b.result=-1;
    uint8_t background[320*200];unsigned start=screen_start(&b);
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++)background[y*320+x]=window->video->planes[x&3][(start+y*80+x/4)&65535];
    unsigned draw_page=(unsigned)c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8);
    (void)JX_CALL(c,0x1832,0x1810,start/0x4000);(void)JX_CALL(c,0x1f2a,0x0718,176);
    for(int i=0;i<SLOTS;i++)inspect(&b,i);
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    while(!b.done) {
        draw_browser(&b,background);JxFarPointer input={0x100,0x9d00};
        for(;;) {
            while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,input.offset,input.segment)!=0){}
            uint16_t type=jx_far_word(c,input.segment,input.offset),key=jx_far_word(c,input.segment,(uint16_t)(input.offset+2));
            if(type==8){jx_music_poll(c);continue;}
            if(type==1 && key==1){b.done=1;break;}
            if(type==1 && (key==0x48 || key==0x50)) {
                int first=b.selected/10*10,count=first==90?9:10;b.selected=first+(b.selected-first+(key==0x48?count-1:1))%count;break;
            }
            if(type==1 && (key==0x4b || key==0x4d || key==0x49 || key==0x51)) {
                int page=(b.selected/10+((key==0x4b || key==0x49)?9:1))%10;b.selected=page*10+b.selected%10;if(b.selected>=99)b.selected=98;break;
            }
            if(type!=0 || (key!=0x1c && key!=0x39))continue;
            inspect(&b,b.selected);Slot *s=&b.slots[b.selected];
            if(!writing && s->state!=1){notice(&b,s->state?L"存档异常":L"空存档");break;}
            if(writing) {
                if(s->state && !overwrite(&b))break;
                int saved=commit_save(&b);if(!saved){notice(&b,L"保存失败");break;}
                if(saved==2)notice(&b,L"预览保存失败");
            }
            last_slot=b.selected;b.result=b.selected;b.done=1;break;
        }
    }
    backdrop(&b,background,0);(void)JX_CALL(c,0x1832,0x1810,draw_page/0x4000);
    (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);(void)JX_CALL0(c,0x1f2a,0xc24e);jx_native_window_present(window);return b.result;
}
