#include "native_hd.h"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <time.h>
static uint8_t memory[1048576],palette[768];
static JxVideoModel video;
static void put16(FILE *f,unsigned n){fputc(n&255,f);fputc((n>>8)&255,f);}
static void put32(FILE *f,uint32_t n){put16(f,n);put16(f,n>>16);}
static void fixture(const char *dir,int bad){
    char path[1024];snprintf(path,sizeof path,"%s/manifest.tsv",dir);FILE *f=fopen(path,"wb");assert(f);
    fputs("# JXHD1\nsprite\t2\t2\tref.idx\timage.bmp\n",f);fclose(f);
    snprintf(path,sizeof path,"%s/ref.idx",dir);f=fopen(path,"wb");assert(f);
    assert(fwrite(palette,1,768,f)==768);const uint8_t src[]={1,2,255,1};assert(fwrite(src,1,4,f)==4);fclose(f);
    snprintf(path,sizeof path,"%s/image.bmp",dir);f=fopen(path,"wb");assert(f);
    if(bad){fputs("broken",f);fclose(f);return;}
    fputs("BM",f);put32(f,54+8*8*4);put32(f,0);put32(f,54);put32(f,40);
    put32(f,8);put32(f,(uint32_t)-8);put16(f,1);put16(f,32);put32(f,0);put32(f,8*8*4);
    for(unsigned i=0;i<4;i++)put32(f,0);
    for(unsigned i=0;i<64;i++)put32(f,0x2255aa);fclose(f);
}
static void pixel(unsigned x,unsigned y,unsigned index){video.planes[x%4][y*80+x/4]=(uint8_t)index;}
static void packed_references(void){
    _mkdir("build/hd-bundle-fixture");_mkdir("build/hd-bundle-fixture/superres");
    fixture("build/hd-bundle-fixture/superres",0);
    FILE *f=fopen("build/hd-bundle-fixture/superres/references.bin","wb");assert(f);
    assert(fwrite("JXREF1\0\0",1,8,f)==8);put32(f,1);assert(fwrite(palette,1,768,f)==768);
    const uint8_t indices[]={1,2,255,1};assert(fwrite(indices,1,4,f)==4);fclose(f);
    f=fopen("build/hd-bundle-fixture/superres/manifest.tsv","wb");assert(f);
    fputs("sprite\t2\t2\t@12\timage.bmp\ttruecolor\n",f);fclose(f);
    JxHd *h=jx_hd_open("build/hd-bundle-fixture");assert(h && jx_hd_asset_count(h)==1);
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};memset(&video,0,sizeof video);memory[0x44e]=memory[0x44f]=0;
    memcpy(memory+0x90000,indices,4);for(unsigned p=0;p<4;p++)pixel(20+p%2,20+p/2,indices[p]);
    uint16_t a[]={20,20,2,2,0,0x9000};jx_hd_blit(h,&c,&video,a,-1);
    assert(jx_hd_render(h,&video,palette)[80*1280+80]==0x2255aa);jx_hd_close(h);
    const char *bad[]={"@0","@4294967295","@-1","@12junk"};
    for(unsigned i=0;i<4;i++){
        f=fopen("build/hd-bundle-fixture/superres/manifest.tsv","wb");assert(f);
        fprintf(f,"sprite\t2\t2\t%s\timage.bmp\n",bad[i]);fclose(f);assert(!jx_hd_open("build/hd-bundle-fixture"));
    }
    memset(&video,0,sizeof video);puts("PASS: standalone superres addon, bundled references and malformed offset rejection");
}
static void cropped_priority(void){
    _mkdir("build/hd-crop-fixture");fixture("build/hd-crop-fixture",0);
    FILE *f=fopen("build/hd-crop-fixture/tall.idx","wb");assert(f);assert(fwrite(palette,1,768,f)==768);
    const uint8_t indices[]={1,2,255,1,3,4,5,6};assert(fwrite(indices,1,8,f)==8);fclose(f);
    f=fopen("build/hd-crop-fixture/tall.bmp","wb");assert(f);
    fputs("BM",f);put32(f,54+8*16*4);put32(f,0);put32(f,54);put32(f,40);put32(f,8);put32(f,(uint32_t)-16);
    put16(f,1);put16(f,32);put32(f,0);put32(f,8*16*4);for(unsigned i=0;i<4;i++)put32(f,0);
    for(unsigned i=0;i<128;i++)put32(f,0x551122);fclose(f);
    f=fopen("build/hd-crop-fixture/manifest.tsv","wb");assert(f);
    fputs("tall\t2\t4\ttall.idx\ttall.bmp\ttruecolor\nexact\t2\t2\tref.idx\timage.bmp\ttruecolor\n",f);fclose(f);
    JxHd *h=jx_hd_open("build/hd-crop-fixture");assert(h && jx_hd_asset_count(h)==2);
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};memcpy(memory+0x90000,indices,8);
    memset(&video,0,sizeof video);memory[0x44e]=memory[0x44f]=0;
    for(unsigned p=0;p<4;p++)pixel(20+p%2,20+p/2,indices[p]);
    uint16_t a[]={20,20,2,2,0,0x9000};jx_hd_blit(h,&c,&video,a,-1);
    assert(jx_hd_render(h,&video,palette)[80*1280+80]==0x551122);jx_hd_close(h);memset(&video,0,sizeof video);
    puts("PASS: taller cropped asset retains manifest priority over an exact-size match");
}
static void planar_tiles(void){
    _mkdir("build/hd-planar-fixture");
    FILE *f=fopen("build/hd-planar-fixture/manifest.tsv","wb");assert(f);
    fputs("tile\t16\t16\tref.idx\timage.bmp\ttruecolor\n",f);fclose(f);
    f=fopen("build/hd-planar-fixture/ref.idx","wb");assert(f);assert(fwrite(palette,1,768,f)==768);
    for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++)fputc(1+(x+3*y)%3,f);fclose(f);
    f=fopen("build/hd-planar-fixture/image.bmp","wb");assert(f);
    fputs("BM",f);put32(f,54+64*64*4);put32(f,0);put32(f,54);put32(f,40);
    put32(f,64);put32(f,(uint32_t)-64);put16(f,1);put16(f,32);put32(f,0);put32(f,64*64*4);
    for(unsigned i=0;i<4;i++)put32(f,0);
    for(unsigned y=0;y<64;y++)for(unsigned x=0;x<64;x++)put32(f,0x224400+x+(y<<16));fclose(f);
    JxHd *h=jx_hd_open("build/hd-planar-fixture");assert(h);
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};memset(&video,0,sizeof video);
    memory[0x44e]=0;memory[0x44f]=0x40;video.start_high=0x40;
    /* Nonzero X lane, second VGA page and a source offset wrapping at FFFF. */
    uint16_t args[]={13,22,19,16,0xff80,0x2000};
    for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++){
        unsigned value=1+(x+3*y)%3,offset=(x%4)*64+y*4+x/4;
        memory[0x20000+(uint16_t)(args[4]+offset)]=(uint8_t)value;
        video.planes[(13+x)%4][0x4000+(22+y)*80+(13+x)/4]=(uint8_t)value;
    }
    jx_hd_blit_planes(h,&c,&video,args);assert(jx_hd_hits(h)==1);
    const uint32_t *out=jx_hd_render(h,&video,palette);
    for(unsigned y=0;y<64;y++)for(unsigned x=0;x<64;x++)
        assert(out[(22*4+y)*1280+13*4+x]==0x224400+x+(y<<16));
    /* Rounded-off width and same-color overwrite must not acquire stale tags. */
    assert(out[22*4*1280+29*4]!=0x224400);
    video.map_mask=1u<<(13%4);jx_hd_write(h,&video,0xa0000+0x4000+22*80+13/4);
    out=jx_hd_render(h,&video,palette);assert(out[22*4*1280+13*4]!=0x224400);
    jx_hd_close(h);memset(&video,0,sizeof video);memory[0x44e]=memory[0x44f]=0;
    puts("PASS: plane-major world tile, lane ordering, page 1, source wrap, rounded width and overwrite");
}
static unsigned plane_pixel(unsigned p){return (p%16%4)*64+(p/16)*4+(p%16)/4;}
static void layered_tiles(void){
    const char *dir="build/hd-planar-fixture";char path[160];
    snprintf(path,sizeof path,"%s/manifest.tsv",dir);FILE *f=fopen(path,"ab");assert(f);
    fputs("actor\t16\t16\tactor.idx\tactor.bmp\ttruecolor\n",f);fclose(f);
    snprintf(path,sizeof path,"%s/actor.idx",dir);f=fopen(path,"wb");assert(f);assert(fwrite(palette,1,768,f)==768);
    for(unsigned p=0;p<256;p++)fputc(p%16>=5 && p%16<10 && p/16>=3 && p/16<13?5:255,f);fclose(f);
    snprintf(path,sizeof path,"%s/actor.bmp",dir);f=fopen(path,"wb");assert(f);
    fputs("BM",f);put32(f,54+64*64*4);put32(f,0);put32(f,54);put32(f,40);put32(f,64);put32(f,(uint32_t)-64);
    put16(f,1);put16(f,32);put32(f,0);put32(f,64*64*4);for(unsigned i=0;i<4;i++)put32(f,0);
    for(unsigned i=0;i<64*64;i++)put32(f,0x7799cc);fclose(f);
    JxHd *h=jx_hd_open(dir);assert(h && jx_hd_asset_count(h)==2);
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};jx_set_word(&c,0x2eb0,0);jx_set_word(&c,0x2eb2,0x8000);
    memset(&video,0,sizeof video);memory[0x44e]=memory[0x44f]=0;
    for(unsigned p=0;p<256;p++){
        memory[0x82000+plane_pixel(p)]=(uint8_t)(1+(p%16+3*(p/16))%3);
        memory[0x81000+plane_pixel(p)]=(uint8_t)(p%16>=5 && p%16<10 && p/16>=3 && p/16<13?5:255);
    }
    uint16_t copy[]={0,0x8000,0,0x8200,256};jx_hd_tile_copy(h,&c,copy,-1);memcpy(memory+0x80000,memory+0x82000,256);
    uint16_t overlay[]={0,0x8000,0,0x8100,256,255};jx_hd_tile_copy(h,&c,overlay,255);
    for(unsigned i=0;i<256;i++)if(memory[0x81000+i]!=255)memory[0x80000+i]=memory[0x81000+i];
    uint16_t draw[]={10,10,16,16,0,0x8000};
    for(unsigned p=0;p<256;p++)pixel(10+p%16,10+p/16,memory[0x80000+plane_pixel(p)]);
    jx_hd_blit_planes(h,&c,&video,draw);const uint32_t *out=jx_hd_render(h,&video,palette);
    assert(out[40*1280+40]==0x224400);assert(out[(10+5)*4*1280+(10+7)*4]==0x7799cc);
    /* An unrecognized foreground must clear just the covered pixels. */
    memset(memory+0x83000,255,256);memory[0x83000+plane_pixel(5*16+7)]=99;
    uint16_t capture[]={10,10,16,16,0,0x8000};jx_hd_tile_capture(h,&c,&video,capture);
    uint16_t unknown[]={0,0x8000,0,0x8300,256,255};jx_hd_tile_copy(h,&c,unknown,255);
    memory[0x80000+plane_pixel(5*16+7)]=99;pixel(17,15,99);
    jx_hd_blit_planes(h,&c,&video,draw);out=jx_hd_render(h,&video,palette);
    assert(out[40*1280+40]==0x224400);assert(out[60*1280+68]==0x808080);
    assert(out[64*1280+68]==0x7799cc);
    assert(jx_hd_export_bmp(h,&video,palette,"build/hd-layered-tiles.bmp"));
    jx_hd_close(h);memset(&video,0,sizeof video);
    puts("PASS: layered world background + actor, transparency, plane capture and unknown foreground fallback");
}
static void original_font(JxHd *h){
    uint8_t glyph[32];for(unsigned i=0;i<32;i++)glyph[i]=(uint8_t)(0x80u>>(i%8));
    FILE *font=fopen("runtime/game/HZK16","rb");
    if(font){
        /* Original HZK16 GB2312 grid: the character 剑 (BD A3). */
        unsigned index=(0xbdu-0xa1u)*94+(0xa3u-0xa1u);
        assert(!fseek(font,(long)(index*32),SEEK_SET));assert(fread(glyph,1,32,font)==32);fclose(font);
    }
    memset(&video,0,sizeof video);jx_hd_reset(h);
    for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++)pixel(40+x,30+y,(glyph[y*2+x/8]&(0x80u>>(x%8)))?1:0);
    const uint32_t *out=jx_hd_render(h,&video,palette);
    for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++){
        unsigned bit=(glyph[y*2+x/8]&(0x80u>>(x%8)))?1:0;
        uint32_t color=((uint32_t)palette[bit*3]*4<<16)|((uint32_t)palette[bit*3+1]*4<<8)|(palette[bit*3+2]*4);
        for(unsigned dy=0;dy<4;dy++)for(unsigned dx=0;dx<4;dx++)assert(out[((30+y)*4+dy)*1280+(40+x)*4+dx]==color);
    }
    assert(jx_hd_export_bmp(h,&video,palette,"build/hd-original-font.bmp"));
    puts("PASS: original HZK16 glyph preserves every source bit as a solid 4x4 block");
    jx_hd_reset(h);
}
static void imported_asset(JxHd *h,JxEventContext *c,const char *key,unsigned w,unsigned hh,const char *preview){
    char path[256];snprintf(path,sizeof path,"runtime/game/hd/references/%s.idx",key);
    FILE *f=fopen(path,"rb");if(!f)return;
    assert(fread(palette,1,768,f)==768);assert(fread(memory+0x40000,1,w*hh,f)==w*hh);fclose(f);
    memset(&video,0,sizeof video);jx_hd_reset(h);
    unsigned sample=0;while(sample<w*hh && memory[0x40000+sample]==255)sample++;assert(sample<w*hh);
    for(unsigned y=0;y<hh;y++)for(unsigned x=0;x<w;x++)pixel(x,y,memory[0x40000+y*w+x]);
    uint16_t a[]={0,0,(uint16_t)w,(uint16_t)hh,0,0x4000};unsigned before=jx_hd_hits(h);
    jx_hd_blit(h,c,&video,a,-1);assert(jx_hd_hits(h)==before+1);
    const uint32_t *out=jx_hd_render(h,&video,palette);
    snprintf(path,sizeof path,"runtime/game/hd/images/%s.bmp",key);f=fopen(path,"rb");assert(f);
    unsigned sx=(sample%w)*4+1,sy=(sample/w)*4+1;uint32_t expected=0;
    assert(!fseek(f,(long)(54+(sy*w*4+sx)*4),SEEK_SET));assert(fread(&expected,4,1,f)==1);fclose(f);
    assert(out[sy*1280+sx]==(expected&0xffffff));assert(jx_hd_export_bmp(h,&video,palette,preview));
    printf("PASS imported asset render: %s\n",key);
}
static void compact_dialogue(JxHd *h,JxEventContext *c){
    const unsigned positions[3][2]={{0,112},{60,0},{30,56}};
    uint8_t glyph[32];memset(glyph,0xaa,sizeof glyph);
    FILE *font=fopen("runtime/game/HZK16","rb");
    if(font){assert(!fseek(font,(long)(((0xbd-0xa1)*94+(0xa3-0xa1))*32),SEEK_SET));assert(fread(glyph,1,32,font)==32);fclose(font);}
    jx_set_word(c,0x2eb0,0x9000);jx_set_word(c,0x2eb2,0x5000);
    for(unsigned test=0;test<3;test++)for(unsigned page=0;page<2;page++){
        unsigned x=positions[test][0],y=positions[test][1],base=page?0x4000:0;
        memset(&video,0,sizeof video);jx_hd_reset(h);
        memset(video.planes,2,sizeof video.planes);
        memory[0x44e]=(uint8_t)base;memory[0x44f]=(uint8_t)(base>>8);
        uint16_t capture[]={(uint16_t)x,(uint16_t)y,260,80,0x9000,0x5000};
        /* Capture must use the draw page even before it becomes visible. */
        jx_hd_dialogue_capture(h,c,&video,palette,capture);
        for(unsigned py=0;py<80;py++)for(unsigned px=0;px<260;px++){
            unsigned index=((px/8+py/8)%2)?3:0;
            memory[0x60d00+py*260+px]=(uint8_t)index;
            video.planes[(x+px)%4][base+(y+py)*80+(x+px)/4]=(uint8_t)index;
        }
        for(unsigned py=0;py<16;py++)for(unsigned px=0;px<16;px++)
            video.planes[(x+10+px)%4][base+(y+2+py)*80+(x+10+px)/4]=1;
        for(unsigned py=0;py<16;py++)for(unsigned px=0;px<16;px++)
            if(glyph[py*2+px/8]&(0x80u>>(px%8)))video.planes[(x+10+px)%4][base+(y+22+py)*80+(x+10+px)/4]=1;
        uint16_t shaded[]={(uint16_t)x,(uint16_t)y,260,80,0x0d00,0x6000};
        jx_hd_blit(h,c,&video,shaded,-1);video.start_high=(uint8_t)(base>>8);
        const uint32_t *out=jx_hd_render(h,&video,palette);
        uint32_t ink=((uint32_t)palette[3]*4<<16)|((uint32_t)palette[4]*4<<8)|(palette[5]*4);
        unsigned left=x*4+(test==0?520:test==1?0:260),top=y*4+80;
        uint32_t background=((uint32_t)palette[6]*4<<16)|((uint32_t)palette[7]*4<<8)|(palette[8]*4);
        for(unsigned py=0;py<32;py++)for(unsigned px=0;px<32;px++)assert(out[(top+4+py)*1280+left+20+px]==ink);
        for(unsigned py=0;py<32;py++)for(unsigned px=0;px<32;px++){
            uint32_t expected=(glyph[(py/2)*2+(px/2)/8]&(0x80u>>((px/2)%8)))?ink:background;
            assert(out[(top+44+py)*1280+left+20+px]==expected);
        }
        assert(out[(top+4)*1280+left+52]==background);assert(out[(top+36)*1280+left+20]==background);
        /* Every unchanged panel pixel must expose the scene, even when the
         * source shaded image contains contrasting image fragments. */
        for(unsigned py=0;py<160;py++)for(unsigned px=100;px<520;px++)assert(out[(top+py)*1280+left+px]==background);
        assert(out[(y*4)*1280+x*4]!=0); /* Old large box restored. */
        assert(jx_hd_render(h,&video,palette)[(top+4)*1280+left+20]==ink); /* Cached frame. */
        if(test==0 && page==0)assert(jx_hd_export_bmp(h,&video,palette,"build/hd-dialogue-32.bmp"));
        jx_hd_blit(h,c,&video,capture,-1);out=jx_hd_render(h,&video,palette);
        assert(out[(y*4+8)*1280+x*4+40]==ink); /* Presentation scope ended. */
    }
    memset(&video,0,sizeof video);jx_hd_reset(h);memory[0x44e]=memory[0x44f]=0;
    puts("PASS: 32px dialogue, portrait-side alignment, transparent background, all placements/pages, cached frame and restore");
}
static void compact_menus(JxHd *h,JxEventContext *c){
    const unsigned layouts[][4]={{0,0,72,80},{76,0,72,100},{100,100,72,40},{100,60,100,60},{248,0,72,60}};
    jx_set_word(c,0x2eb0,0x9000);jx_set_word(c,0x2eb2,0x5000);
    for(unsigned test=0;test<sizeof layouts/sizeof layouts[0];test++){
        unsigned x=layouts[test][0],y=layouts[test][1],w=layouts[test][2],hh=layouts[test][3];
        memset(&video,0,sizeof video);jx_hd_reset(h);memset(video.planes,2,sizeof video.planes);
        memory[0x44e]=memory[0x44f]=0;
        uint16_t capture[]={(uint16_t)x,(uint16_t)y,(uint16_t)w,(uint16_t)hh,0x9000,0x5000};
        jx_hd_dialogue_capture(h,c,&video,palette,capture);
        unsigned sum=0x9000+w*hh,segment=0x5000+(sum>>16)*0x1000;
        uint16_t shaded[]={(uint16_t)x,(uint16_t)y,(uint16_t)w,(uint16_t)hh,(uint16_t)sum,(uint16_t)segment};
        unsigned address=segment*16+(uint16_t)sum;
        memset(memory+address,0,w*hh);
        for(unsigned py=0;py<hh;py++)for(unsigned px=0;px<w;px++)pixel(x+px,y+py,0);
        jx_hd_blit(h,c,&video,shaded,-1);
        for(unsigned py=0;py<16;py++)for(unsigned px=0;px<16;px++)pixel(x+4+px,y+1+py,1);
        /* Two-choice marker extends beyond the 72px background rectangle. */
        if(test==2)for(unsigned py=0;py<16;py++)for(unsigned px=0;px<3;px++)pixel(x+78+px,y+1+py,1);
        const uint32_t *out=jx_hd_render(h,&video,palette);
        uint32_t ink=((uint32_t)palette[3]*4<<16)|((uint32_t)palette[4]*4<<8)|(palette[5]*4);
        uint32_t background=((uint32_t)palette[6]*4<<16)|((uint32_t)palette[7]*4<<8)|(palette[8]*4);
        for(unsigned py=0;py<32;py++)for(unsigned px=0;px<32;px++)assert(out[(y*4+2+py)*1280+x*4+8+px]==ink);
        assert(out[(y*4+2)*1280+x*4+40]==background);
        if(test==2)assert(out[(y*4+2)*1280+x*4+156]==ink);
        jx_hd_blit(h,c,&video,capture,-1);out=jx_hd_render(h,&video,palette);
        assert(out[(y*4+4)*1280+x*4+16]==ink);
    }
    memset(&video,0,sizeof video);jx_hd_reset(h);memset(video.planes,2,sizeof video.planes);
    jx_hd_ui_begin(h,&video,palette,1);
    memset(video.planes,0,sizeof video.planes);
    for(unsigned py=0;py<16;py++)for(unsigned px=0;px<16;px++)pixel(4+px,18+py,1);
    const uint32_t *out=jx_hd_render(h,&video,palette);
    uint32_t ink=((uint32_t)palette[3]*4<<16)|((uint32_t)palette[4]*4<<8)|(palette[5]*4);
    for(unsigned py=0;py<32;py++)for(unsigned px=0;px<32;px++)assert(out[(200+36+py)*1280+320+8+px]==ink);
    assert(out[200*1280+320]==0);assert(out[0]!=0);
    jx_hd_ui_begin(h,&video,palette,0);out=jx_hd_render(h,&video,palette);
    assert(out[236*1280+328]==ink); /* Nested overwrite has no second scale. */
    jx_hd_ui_end(h);out=jx_hd_render(h,&video,palette);assert(out[236*1280+328]==ink);
    jx_hd_ui_end(h);out=jx_hd_render(h,&video,palette);assert(out[72*1280+16]==ink);
    jx_hd_ui_end(h);jx_hd_reset(h);memset(&video,0,sizeof video);
    puts("PASS: main/system/yes-no/three-choice/right-edge menus, extended cursor, 640x400 browser, nested UI and restore");
}
static void compact_item_notice(JxHd *h,JxEventContext *c){
    jx_set_word(c,0x2eb0,0x9000);jx_set_word(c,0x2eb2,0x5000);
    for(unsigned page=0;page<2;page++){
        memset(&video,0,sizeof video);jx_hd_reset(h);memset(video.planes,2,sizeof video.planes);
        unsigned base=page*0x4000;memory[0x44e]=0;memory[0x44f]=(uint8_t)(base>>8);
        uint16_t capture[]={100,100,160,17,0x9000,0x5000};
        jx_hd_dialogue_capture(h,c,&video,palette,capture);
        /* Direct font raster writes: no menu shading or text dispatch. */
        for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++)video.planes[x%4][base+(100+y)*80+25+x/4]=1;
        video.start_high=(uint8_t)(base>>8);
        const uint32_t *out=jx_hd_render(h,&video,palette);
        uint32_t ink=((uint32_t)palette[3]*4<<16)|((uint32_t)palette[4]*4<<8)|(palette[5]*4);
        uint32_t background=((uint32_t)palette[6]*4<<16)|((uint32_t)palette[7]*4<<8)|(palette[8]*4);
        for(unsigned y=0;y<32;y++)for(unsigned x=0;x<32;x++)assert(out[(400+y)*1280+400+x]==ink);
        for(unsigned y=0;y<68;y++)for(unsigned x=32;x<640;x++)assert(out[(400+y)*1280+400+x]==background);
        assert(out[432*1280+400]==background);
        assert(jx_hd_render(h,&video,palette)[400*1280+400]==ink);
        jx_hd_blit(h,c,&video,capture,-1);out=jx_hd_render(h,&video,palette);assert(out[460*1280+460]==ink);
    }
    memset(&video,0,sizeof video);jx_hd_reset(h);memory[0x44e]=memory[0x44f]=0;
    puts("PASS: direct-raster item notification is 32px, transparent, on both VGA pages, cached and restored");
}
int main(void){
    for(unsigned i=0;i<768;i++)palette[i]=32;
    memset(palette,0,3);palette[3]=16;palette[4]=32;palette[5]=48;
    _mkdir("build");_mkdir("build/hd-fixture");_mkdir("build/hd-bad-fixture");
    fixture("build/hd-fixture",0);fixture("build/hd-bad-fixture",1);
    packed_references();
    cropped_priority();
    planar_tiles();
    layered_tiles();
    assert(!jx_hd_open("build/no-such-hd-pack"));
    JxHd *h=jx_hd_open("build/hd-fixture");assert(h && jx_hd_asset_count(h)==1);
    original_font(h);
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};
    compact_dialogue(h,&c);
    compact_menus(h,&c);
    compact_item_notice(h,&c);
    memory[0x2000]=1;memory[0x2001]=2;memory[0x2002]=255;memory[0x2003]=1;
    uint16_t a[]={10,12,2,2,0,0x200};
    pixel(10,12,1);pixel(11,12,2);pixel(10,13,255);pixel(11,13,1);
    jx_hd_blit(h,&c,&video,a,-1);assert(jx_hd_hits(h)==1);
    const uint32_t *out=jx_hd_render(h,&video,palette);assert(out[48*1280+40]==0x2255aa);
    /* Actual external BMP detail is visible, rather than a stretched source. */
    video.map_mask=1u<<(10%4);jx_hd_write(h,&video,0xa0000+12*80+10/4);
    out=jx_hd_render(h,&video,palette);assert(out[48*1280+40]!=0x2255aa);
    jx_hd_blit(h,&c,&video,a,-1);
    /* VGA latch copies retain high-resolution provenance. */
    unsigned from=12*80+10/4,to=20*80+10/4;jx_hd_read(h,0xa0000+from);
    video.mode=1;jx_hd_write(h,&video,0xa0000+to);video.planes[2][to]=1;video.mode=0;
    out=jx_hd_render(h,&video,palette);assert(out[80*1280+40]==0x2255aa);
    /* Capture then restore to a different place keeps HD source coordinates. */
    memory[0x3000]=1;memory[0x3001]=255;
    uint16_t capture[]={10,12,1,2,0,0x300};jx_hd_capture(h,&c,capture);
    uint16_t restore[]={30,30,1,2,0,0x300};
    pixel(30,30,1);pixel(30,31,255);
    jx_hd_blit(h,&c,&video,restore,-1);
    uint8_t faded[768];for(unsigned i=0;i<768;i++)faded[i]=palette[i]/2;
    out=jx_hd_render(h,&video,faded);assert(out[120*1280+120]==0x112a55);
    memset(faded,0,768);out=jx_hd_render(h,&video,faded);assert(out[120*1280+120]==0);
    /* Same-address captures must restore the latest provenance, even if all
     * low-resolution indices are identical after a same-color overwrite. */
    video.map_mask=1u<<(10%4);jx_hd_write(h,&video,0xa0000+12*80+10/4);
    jx_hd_capture(h,&c,capture);restore[0]=32;
    pixel(32,30,1);pixel(32,31,255);jx_hd_blit(h,&c,&video,restore,-1);
    out=jx_hd_render(h,&video,palette);assert(out[120*1280+128]!=0x2255aa);
    /* A transparent source must not cover already present pixels. */
    pixel(50,50,1);pixel(51,50,2);pixel(50,51,2);pixel(51,51,1);
    uint16_t transparent[]={50,50,2,2,0,0x200};jx_hd_blit(h,&c,&video,transparent,255);
    out=jx_hd_render(h,&video,palette);assert(out[204*1280+200]!=0x2255aa);
    jx_hd_reset(h);out=jx_hd_render(h,&video,palette);assert(out[120*1280+120]!=0x2255aa);
    jx_hd_close(h);h=jx_hd_open("build/hd-bad-fixture");assert(h);
    jx_hd_blit(h,&c,&video,a,-1);out=jx_hd_render(h,&video,palette);assert(out[48*1280+40]!=0x2255aa);jx_hd_close(h);
    /* User artwork keeps its RGB hue, including detail over an original black
     * index, but still participates in global fades. Older five-column packs
     * above retain their original per-index palette behavior. */
    _mkdir("build/hd-truecolor-fixture");fixture("build/hd-truecolor-fixture",0);
    FILE *f=fopen("build/hd-truecolor-fixture/manifest.tsv","wb");assert(f);
    fputs("sprite\t2\t2\tref.idx\timage.bmp\ttruecolor\n",f);fclose(f);
    f=fopen("build/hd-truecolor-fixture/ref.idx","r+b");assert(f);assert(!fseek(f,-1,SEEK_END));fputc(0,f);fclose(f);
    memory[0x2003]=0;pixel(11,13,0);
    h=jx_hd_open("build/hd-truecolor-fixture");assert(h);jx_hd_blit(h,&c,&video,a,-1);
    out=jx_hd_render(h,&video,palette);assert(out[52*1280+44]==0x2255aa);
    uint8_t hue[768];memcpy(hue,palette,768);hue[3]=48;hue[5]=16;
    out=jx_hd_render(h,&video,hue);assert(out[48*1280+40]==0x2255aa);
    for(unsigned i=0;i<768;i++)hue[i]=palette[i]/2;
    out=jx_hd_render(h,&video,hue);assert(out[48*1280+40]==0x112a55);
    memset(hue,0,768);out=jx_hd_render(h,&video,hue);assert(out[48*1280+40]==0);jx_hd_close(h);
    /* A real extracted background proves the pack matches actual game pixels. */
    h=jx_hd_open("runtime/game/hd");if(h){
        FILE *f=fopen("runtime/game/hd/references/BEGIN_IMG_000.idx","rb");assert(f);
        assert(fread(palette,1,768,f)==768);assert(fread(memory+0x40000,1,64000,f)==64000);fclose(f);
        for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++)pixel(x,y,memory[0x40000+y*320+x]);
        uint16_t full[]={0,0,320,200,0,0x4000};jx_hd_blit(h,&c,&video,full,-1);assert(jx_hd_hits(h)==1);
        clock_t begin=clock();for(unsigned i=0;i<10;i++)assert(jx_hd_render(h,&video,palette));
        printf("Real pack: %u assets, external background recognized; 10 frames %.1f ms\n",jx_hd_asset_count(h),1000.0*(clock()-begin)/CLOCKS_PER_SEC);
        imported_asset(h,&c,"FACE_INF_089",60,70,"build/hd-import-face.bmp");
        imported_asset(h,&c,"FIGHT_ZD000_IMG_000",320,200,"build/hd-import-background.bmp");
        imported_asset(h,&c,"FIGHT_SHADE_00_IMG_000",220,148,"build/hd-import-shade.bmp");
        jx_hd_close(h);
    }
    puts("PASS: external detail, same-index overwrite, VGA copy, transparency, palette fade, reset and corrupt fallback");
    return 0;
}
