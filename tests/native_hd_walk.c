#include "native_hd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
static unsigned char memory[1048576],palette[768];static JxVideoModel video;
static void put32(FILE *f,unsigned v){for(unsigned i=0;i<4;i++)fputc((int)(v>>(i*8))&255,f);}
static void put16(FILE *f,unsigned v){fputc((int)v&255,f);fputc((int)(v>>8)&255,f);}
static unsigned off(unsigned p){return (p%16%4)*64+(p/16)*4+p%16/4;}
static void show(JxHd *h,JxEventContext *c,unsigned x,unsigned y){
 for(unsigned p=0;p<256;p++)video.planes[(x+p%16)%4][(y+p/16)*80+(x+p%16)/4]=memory[0x80000+off(p)];
 uint16_t draw[]={(uint16_t)x,(uint16_t)y,16,16,0,0x8000};jx_hd_blit_planes(h,c,&video,draw);
}
int main(void){
 remove("build/hd-walk-fixture/walk-redraw/walk_164.bin.wide");
 remove("build/hd-walk-fixture/walk-redraw/manifest.tsv");
 remove("build/hd-walk-fixture/walk-redraw/geometry.bin");
 _mkdir("build/hd-walk-fixture");_mkdir("build/hd-walk-fixture/walk-redraw");
 palette[3]=63;palette[4]=63;palette[5]=63;palette[8]=63;palette[10]=63;
 FILE *f=fopen("build/hd-walk-fixture/manifest.tsv","wb");assert(f);fputs("background\t16\t16\tref.idx\timage.bmp\n",f);fclose(f);
 f=fopen("build/hd-walk-fixture/ref.idx","wb");assert(f);fwrite(palette,1,768,f);for(unsigned i=0;i<256;i++)fputc(2,f);fclose(f);
 f=fopen("build/hd-walk-fixture/image.bmp","wb");assert(f);fputs("BM",f);put32(f,54+64*64*4);put32(f,0);put32(f,54);put32(f,40);put32(f,64);put32(f,(unsigned)-64);fputc(1,f);fputc(0,f);fputc(32,f);fputc(0,f);put32(f,0);put32(f,64*64*4);for(unsigned i=0;i<4;i++)put32(f,0);for(unsigned i=0;i<4096;i++)put32(f,0xff0000ff);fclose(f);
 f=fopen("build/hd-walk-fixture/walk-redraw/walk_164.bin","wb");assert(f);fwrite("JXWALK1\0",1,8,f);put32(f,60);fwrite(palette,1,768,f);
 for(unsigned tile=0;tile<60;tile++)for(unsigned y=0;y<64;y++)for(unsigned x=0;x<64;x++)put32(f,tile==4?0:tile?0xff00ff00:x>=4 && x<8?0:x>=8 && x<12?0x80ff0000:0xffff0000);fclose(f);
 JxHd *h=jx_hd_open("build/hd-walk-fixture");assert(h);JxEventContext c={memory,0,NULL,NULL,NULL,NULL};jx_set_word(&c,0x2eb2,0x8000);
 memset(memory+0x82000,2,256);memset(memory+0x83000,255,256);memory[0x83000+off(1)]=1;
 uint16_t bg[]={0,0x8000,0,0x8200,256},sprite[]={0,0x8000,0,0x8300,256,255};
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);
 jx_hd_walk_slot(h,30,164,100);jx_hd_walk_source(h,1,100);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,16,16);
 const uint32_t *out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff0000);assert(out[64*1280+68]==0x0000ff);assert(out[64*1280+72]==0x80007f);assert(jx_hd_walk_hits(h)==1);
 /* Capture/restore and VGA page copies retain both artwork and its underlay. */
 for(unsigned p=0;p<256;p++)memory[0x90000+p]=video.planes[(16+p%16)%4][(16+p/16)*80+(16+p%16)/4];
 jx_hd_capture(h,&c,(uint16_t[]){16,16,16,16,0,0x9000});
 for(unsigned p=0;p<256;p++)video.planes[(32+p%16)%4][(32+p/16)*80+(32+p%16)/4]=memory[0x90000+p];
 jx_hd_blit(h,&c,&video,(uint16_t[]){32,32,16,16,0,0x9000},-1);out=jx_hd_render(h,&video,palette);
 assert(out[128*1280+128]==0xff0000 && out[128*1280+132]==0xff);
 unsigned source=16*80+4,dest=48*80+4;jx_hd_read(h,0xa0000+source);
 for(unsigned lane=0;lane<4;lane++)video.planes[lane][dest]=video.planes[lane][source];
 video.mode=1;video.map_mask=15;jx_hd_write(h,&video,0xa0000+dest);video.mode=0;
 out=jx_hd_render(h,&video,palette);assert(out[192*1280+64]==0xff0000 && out[192*1280+68]==0xff);
 /* A foreground layer still occludes the widened artwork. */
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});memset(memory+0x84000,255,256);memory[0x84000]=3;
 jx_hd_tile_copy(h,&c,(uint16_t[]){0,0x8000,0,0x8400,256,255},255);memory[0x80000]=3;show(h,&c,16,16);out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0x00fc00);assert(out[64*1280+68]==0xff);
 /* Opaque redraw removes the previous pose completely. */
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);show(h,&c,16,16);out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff);
 jx_hd_walk_source(h,1,101);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,16,16);out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0x00ff00);
 unsigned char black[768]={0};out=jx_hd_render(h,&video,black);assert(out[64*1280+64]==0);jx_hd_close(h);
 /* Multiple identities, differing frame counts, and exact aliases are independent. */
 f=fopen("build/hd-walk-fixture/walk-redraw/walk_127.bin","wb");assert(f);fwrite("JXWALK1\0",1,8,f);put32(f,36);fwrite(palette,1,768,f);
 for(unsigned i=0;i<36*4096;i++)put32(f,i<4096?0:0xffffff00);fclose(f);
 f=fopen("build/hd-walk-fixture/walk-redraw/manifest.tsv","wb");assert(f);
 fputs("164\t60\twalk_164.bin\n127\t36\twalk_127.bin\n1001\t60\twalk_164.bin\n999\t4097\ttoo-large.bin\n998\t1\t../escape.bin\n",f);fclose(f);
 f=fopen("build/hd-walk-fixture/walk-redraw/geometry.bin","wb");assert(f);fwrite("JXWGEO1\0",1,8,f);put32(f,2);
 put16(f,164);put16(f,60);put16(f,3);fputc(0,f);fputc(0,f);
 for(unsigned i=0;i<60;i++)put16(f,i==3?1:i==4?0:i%3);
 put16(f,127);put16(f,36);put16(f,3);fputc(3,f);fputc(7,f);for(unsigned i=0;i<36;i++)put16(f,i%3);fclose(f);
 h=jx_hd_open("build/hd-walk-fixture");assert(h && jx_hd_walk_resources(h)==2);
 memset(&video,0,sizeof video);jx_hd_walk_slot(h,30,164,100);jx_hd_walk_slot(h,31,127,200);jx_hd_walk_slot(h,32,1001,500);
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);
 jx_hd_walk_source(h,1,235);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xffff00);
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);
 jx_hd_walk_source(h,1,500);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,32,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+128]==0xff0000 && out[64*1280+64]==0xffff00);
 unsigned hits=jx_hd_walk_hits(h);jx_hd_walk_source(h,0,100);
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff && jx_hd_walk_hits(h)==hits);
 /* Three expanded sprites overlap although their original tile is empty. */
 memset(memory+0x85000,255,256);uint16_t empty_sprite[]={0,0x8000,0,0x8500,256,255};
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);
 jx_hd_walk_source(h,1,235);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 for(unsigned layer=0;layer<2;layer++){
  jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});
  jx_hd_walk_source(h,1,layer?500:100);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
  out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff0000 && out[64*1280+68]==0xffff00);
  assert(out[64*1280+72]==(layer?0xff3f00:0xff7f00));
 }
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,500);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+72]==0xff3f00); /* Same draw is idempotent. */
 for(unsigned p=0;p<256;p++)memory[0x91000+p]=2;
 jx_hd_capture(h,&c,(uint16_t[]){16,16,16,16,0,0x9100});
 for(unsigned p=0;p<256;p++)video.planes[(64+p%16)%4][(16+p/16)*80+(64+p%16)/4]=2;
 jx_hd_blit(h,&c,&video,(uint16_t[]){64,16,16,16,0,0x9100},-1);out=jx_hd_render(h,&video,palette);
 assert(out[64*1280+260]==0xffff00 && out[64*1280+264]==0xff3f00);
 jx_hd_read(h,0xa0000+16*80+4);for(unsigned lane=0;lane<4;lane++)video.planes[lane][80*80+4]=2;
 video.mode=1;video.map_mask=15;jx_hd_write(h,&video,0xa0000+80*80+4);video.mode=0;
 out=jx_hd_render(h,&video,palette);assert(out[320*1280+68]==0xffff00 && out[320*1280+72]==0xff3f00);
 out=jx_hd_render(h,&video,black);assert(out[64*1280+68]==0 && out[64*1280+72]==0);
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,504);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+72]==0xff7f00); /* Non-sequential tile 4 replaces the old pose. */
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,500);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 jx_hd_walk_slot(h,32,127,500);jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,500);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+72]==0xff7f00); /* Graphic change on the same source slot. */
 jx_hd_walk_slot(h,32,164,500);jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,500);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 uint16_t environment[]={0,0x8000,0,0x8600,256,255};memset(memory+0x86000,212,256);
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,200);jx_hd_tile_copy(h,&c,environment,255);memcpy(memory+0x80000,memory+0x86000,256);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff && out[64*1280+72]==0xff);
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,100);jx_hd_tile_copy(h,&c,empty_sprite,255);show(h,&c,16,16);
 memset(memory+0x86000,7,256);jx_hd_tile_capture(h,&c,&video,(uint16_t[]){16,16,16,16,0,0x8000});jx_hd_walk_source(h,1,200);jx_hd_tile_copy(h,&c,environment,255);memcpy(memory+0x80000,memory+0x86000,256);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff && out[64*1280+72]==0xff);jx_hd_close(h);
 puts("PASS: repeated draws, pose/graphic replacement, arbitrary tile order and scene-palette background patches");
 puts("PASS: three overlapping characters, partial alpha, capture matching an ordinary background, VGA latches and fade");
 /* Optional wide artwork extends the original footprint without stretching.
  * Native foreground coverage and page/capture copies also apply outside it. */
 f=fopen("build/hd-walk-fixture/walk-redraw/walk_164.bin.wide","wb");assert(f);
 fwrite("JXWIDE1\0",1,8,f);put32(f,60);put32(f,96);
 for(unsigned t=0;t<60;t++)for(unsigned y=0;y<64;y++)for(unsigned x=0;x<96;x++)put32(f,t?0:x<16?0xff00ffff:x>=80?0xffff00ff:0);fclose(f);
 h=jx_hd_open("build/hd-walk-fixture");assert(h);memset(&video,0,sizeof video);jx_set_word(&c,0x44e,0);
 jx_hd_walk_slot(h,30,164,100);
 for(unsigned x=0;x<48;x+=16){jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);show(h,&c,x,16);}
 jx_hd_walk_source(h,1,100);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+48]==0x00ffff && out[64*1280+143]==0xff00ff);
 /* Foreground lives in the neighboring map cell, beyond the old 64px core. */
 jx_hd_tile_capture(h,&c,&video,(uint16_t[]){0,16,16,16,0,0x8000});
 for(unsigned p=0;p<256;p++)memory[0x80000+off(p)]=video.planes[(p%16)%4][(16+p/16)*80+(p%16)/4];
 memset(memory+0x84000,255,256);memory[0x84000+off(12)]=3;
 jx_hd_tile_copy(h,&c,(uint16_t[]){0,0x8000,0,0x8400,256,255},255);memory[0x80000+off(12)]=3;show(h,&c,0,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+48]==0x00fc00 && out[64*1280+52]==0x00ffff);
 /* Captured foreground mask is restored with its original indexed pixels. */
 for(unsigned p=0;p<256;p++)memory[0x92000+p]=video.planes[(p%16)%4][(16+p/16)*80+(p%16)/4];
 jx_hd_capture(h,&c,(uint16_t[]){0,16,16,16,0,0x9200});
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);show(h,&c,0,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+48]==0x00ffff);
 for(unsigned p=0;p<256;p++)video.planes[(p%16)%4][(16+p/16)*80+(p%16)/4]=memory[0x92000+p];
 jx_hd_blit(h,&c,&video,(uint16_t[]){0,16,16,16,0,0x9200},-1);out=jx_hd_render(h,&video,palette);assert(out[64*1280+48]==0x00fc00);
 /* A page shift translates the footprint and its derived wide margins. */
 for(unsigned at=0;at<16000;at++){jx_hd_read(h,0xa0000+at);for(unsigned lane=0;lane<4;lane++)video.planes[lane][20480+at]=video.planes[lane][at];video.mode=1;video.map_mask=15;jx_hd_write(h,&video,0xa0000+20480+at);}video.mode=0;
 video.start_high=80;out=jx_hd_render(h,&video,palette);assert(out[64*1280+48]==0x00fc00 && out[64*1280+143]==0xff00ff);
 out=jx_hd_render(h,&video,black);assert(out[64*1280+143]==0);video.start_high=0;
 /* Erasing only the original core also removes every extended margin. */
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+52]==0xff && out[64*1280+143]==0xff);jx_hd_close(h);
 remove("build/hd-walk-fixture/walk-redraw/walk_164.bin.wide");
 puts("PASS: wide margins, neighboring foreground, capture/restore, page copies, fade and complete erasure");
 /* A visible model survives cache pressure; an offscreen one is reloaded. */
 f=fopen("build/hd-walk-fixture/walk-redraw/geometry.bin","wb");assert(f);fwrite("JXWGEO1\0",1,8,f);put32(f,1);put16(f,0);put16(f,4097);put16(f,1);put16(f,0);fclose(f); /* Malformed optional geometry is ignored. */
 f=fopen("build/hd-walk-fixture/walk-redraw/manifest.tsv","wb");assert(f);
 for(unsigned i=0;i<65;i++)fprintf(f,"%u\t64\tcache_%02u.bin\n",i,i);fclose(f);
 for(unsigned i=0;i<65;i++){
  char path[160];snprintf(path,sizeof path,"build/hd-walk-fixture/walk-redraw/cache_%02u.bin",i);
  f=fopen(path,"wb");assert(f);fwrite("JXWALK1\0",1,8,f);put32(f,64);fwrite(palette,1,768,f);
  for(unsigned p=0;p<64*4096;p++)put32(f,i?0xff00ff00:0xffff0000);fclose(f);
 }
 h=jx_hd_open("build/hd-walk-fixture");assert(h && jx_hd_walk_resources(h)==65);memset(&video,0,sizeof video);
 jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);
 jx_hd_walk_slot(h,30,0,100);jx_hd_walk_source(h,1,100);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff0000);
 f=fopen("build/hd-walk-fixture/walk-redraw/cache_00.bin","r+b");assert(f);assert(!fseek(f,780,SEEK_SET));put32(f,0xff00ffff);fclose(f);
 for(unsigned i=1;i<65;i++){jx_hd_walk_slot(h,31,i,1000+i*64);assert(jx_hd_walk_cache_bytes(h)<=64u*1024*1024);}
 jx_hd_write(h,&video,0xaffff);out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xff0000);
 jx_hd_walk_source(h,0,0);jx_hd_tile_copy(h,&c,bg,-1);memcpy(memory+0x80000,memory+0x82000,256);show(h,&c,16,16);(void)jx_hd_render(h,&video,palette);
 for(unsigned i=1;i<65;i++){jx_hd_walk_slot(h,31,i,1000+i*64);assert(jx_hd_walk_cache_bytes(h)<=64u*1024*1024);}
 jx_hd_walk_slot(h,30,0,100);jx_hd_walk_source(h,1,100);jx_hd_tile_copy(h,&c,sprite,255);memory[0x80000+off(1)]=1;show(h,&c,16,16);
 out=jx_hd_render(h,&video,palette);assert(out[64*1280+64]==0xffff);jx_hd_close(h);
 puts("PASS: visible character remains pinned under cache pressure; offscreen artwork is evicted and reloaded");
 puts("PASS: multiple character identities, unequal frame counts, alias reuse, invalid/path-traversal manifests and ordinary-map isolation");
 puts("PASS: supplied alpha expands beyond old mask, old silhouette is erased, partial alpha blends, foreground occludes, pose changes, redraw clears and fade reaches black");return 0;
}
