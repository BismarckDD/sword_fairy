#include "native_hd.h"
#undef NDEBUG
#include <assert.h>
#include <direct.h>
#include <stdio.h>
#include <string.h>
static uint8_t memory[1048576],palette[768],tile[256];
static JxVideoModel video;
static const char *root="build/hd-map-fixture";
static FILE *file(const char *name){char p[256];snprintf(p,sizeof p,"%s/%s",root,name);FILE *f=fopen(p,"wb");assert(f);return f;}
static void u16(FILE *f,unsigned n){fputc(n&255,f);fputc((n>>8)&255,f);}
static void u32(FILE *f,uint32_t n){u16(f,n);u16(f,n>>16);}
static void bmp(const char *name,unsigned w,unsigned h,uint32_t left,uint32_t right){
    FILE *f=file(name);fputs("BM",f);u32(f,54+w*h*4);u32(f,0);u32(f,54);u32(f,40);u32(f,w);u32(f,(uint32_t)-(int)h);
    u16(f,1);u16(f,32);u32(f,0);u32(f,w*h*4);for(unsigned i=0;i<4;i++)u32(f,0);
    for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++)u32(f,x<w/2?left:right);fclose(f);
}
static void ref(const char *name,unsigned w,unsigned h,int foreground){
    FILE *f=file(name);assert(fwrite(palette,1,768,f)==768);
    for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++)fputc(foreground?(x%16>=8?7:255):tile[(y%16)*16+x%16],f);fclose(f);
}
static unsigned plane(unsigned p){return (p%16%4)*64+(p/16)*4+p%16/4;}
static void draw(JxHd *hd,JxEventContext *c,unsigned x,int foreground){
    uint16_t copy[]={0,0x8000,0,0x8200,256};
    for(unsigned p=0;p<256;p++)memory[0x82000+plane(p)]=tile[p];
    jx_hd_tile_copy(hd,c,copy,-1);memcpy(memory+0x80000,memory+0x82000,256);
    if(foreground){
        for(unsigned p=0;p<256;p++)memory[0x82000+plane(p)]=(uint8_t)(p%16>=8?7:255);
        jx_hd_tile_copy(hd,c,copy,255);
        for(unsigned p=0;p<256;p++)if(p%16>=8)memory[0x80000+plane(p)]=7;
    }
    for(unsigned p=0;p<256;p++){unsigned px=x+p%16,y=p/16;video.planes[px%4][y*80+px/4]=memory[0x80000+plane(p)];}
    uint16_t blit[]={(uint16_t)x,0,16,16,0,0x8000};jx_hd_blit_planes(hd,c,&video,blit);
}
int main(void){
    for(unsigned i=0;i<256;i++)palette[i*3]=palette[i*3+1]=palette[i*3+2]=63;
    for(unsigned p=0;p<256;p++)tile[p]=(uint8_t)(1+p%3);
    _mkdir(root);_mkdir("build/hd-map-fixture/maps");_mkdir("build/hd-map-fixture/maps/imagegen");
    FILE *f=file("manifest.tsv");fputs("TILE_bg\t16\t16\tbase.idx\tbase.bmp\ttruecolor\nTILE_fg\t16\t16\tfg.idx\tfg.bmp\ttruecolor\n",f);fclose(f);
    ref("base.idx",16,16,0);ref("fg.idx",16,16,1);ref("region.idx",32,32,0);ref("region_fg.idx",32,32,1);
    bmp("base.bmp",64,64,0x112233,0x112233);bmp("fg.bmp",64,64,0x334455,0x334455);
    bmp("region.bmp",128,128,0x285078,0xb08040);bmp("region_fg.bmp",128,128,0x007733,0x990055);
    f=file("maps/imagegen/scene-manifest.tsv");
    fputs("TEST\t0\t0\t32\t32\tregion.idx\tregion.bmp\nTEST\t0\t0\t32\t32\tregion_fg.idx\tregion_fg.bmp\n"
          "BAD\t0\t0\t321\t16\tregion.idx\tregion.bmp\nBAD\t0\t0\t32\t16\t../outside.idx\tregion.bmp\n"
          "BADREF\t0\t0\t16\t16\tbroken.idx\tbase.bmp\n",f);fclose(f);
    f=file("broken.idx");fputc(0,f);fclose(f);
    JxHd *hd=jx_hd_open(root);assert(hd && jx_hd_asset_count(hd)==5);
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};jx_set_word(&c,0x2eb0,0);jx_set_word(&c,0x2eb2,0x8000);memcpy(memory+0x9356,"TEST",5);
    draw(hd,&c,0,0);draw(hd,&c,16,0);const uint32_t *out=jx_hd_render(hd,&video,palette);
    assert(out[4*1280+4]==0x285078 && out[4*1280+68]==0xb08040);
    draw(hd,&c,0,1);draw(hd,&c,16,1);out=jx_hd_render(hd,&video,palette);
    assert(out[4*1280+4]==0x285078 && out[4*1280+36]==0x007733);
    assert(out[4*1280+68]==0xb08040 && out[4*1280+100]==0x990055);
    memcpy(memory+0x9356,"test",5);jx_set_word(&c,0x8896,1);jx_set_word(&c,0x8898,1);
    draw(hd,&c,0,0);out=jx_hd_render(hd,&video,palette);assert(out[4*1280+4]==0xb08040);
    memset(palette,0,sizeof palette);out=jx_hd_render(hd,&video,palette);assert(out[4*1280+4]==0);
    memset(palette,63,sizeof palette);
    memcpy(memory+0x9356,"OTHER",6);draw(hd,&c,0,0);out=jx_hd_render(hd,&video,palette);assert(out[4*1280+4]==0x112233);
    memcpy(memory+0x9356,"BADREF",7);draw(hd,&c,0,0);out=jx_hd_render(hd,&video,palette);assert(out[4*1280+4]==0x112233);
    memcpy(memory+0x9356,"TEST",5);jx_set_word(&c,0x8896,0);jx_set_word(&c,0x8898,0);draw(hd,&c,0,0);
    video.map_mask=15;video.mode=1;jx_hd_read(hd,0xa0000);
    for(unsigned lane=0;lane<4;lane++)video.planes[lane][80]=video.planes[lane][0];
    jx_hd_write(hd,&video,0xa0000+80);video.mode=0;out=jx_hd_render(hd,&video,palette);assert(out[4*1280]==0x285078);
    /* An unrecognized direct pixel must erase the region tag normally. */
    video.planes[1][0]=8;video.map_mask=2;jx_hd_write(hd,&video,0xa0000);out=jx_hd_render(hd,&video,palette);
    assert(out[4]==0xfcfcfc);
    jx_hd_close(hd);puts("PASS: position-specific scene artwork, repeated tile identities, foreground masks, camera offset, map isolation, invalid regions and direct overwrite");
    return 0;
}
