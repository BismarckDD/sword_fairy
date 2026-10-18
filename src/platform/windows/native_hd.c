#include "native_hd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HD_ASSETS 65534
#define HD_BUCKETS 131072
#define HD_CAPTURES 12
#define HD_CACHE_BYTES (128u*1024u*1024u)
typedef struct {
    unsigned width,height;
    uint32_t hash;
    uint8_t *indices,palette[768];
    uint32_t *pixels;
    char image[240];
    int failed;
    unsigned used;
    int truecolor,dynamic_kind;
    unsigned next_hash,next_width;
} Asset;
typedef struct {unsigned address,width,height;uint8_t *indices;uint32_t *tags;} Capture;
struct JxHd {
    Asset assets[HD_ASSETS];unsigned count,hits,tile_hits,animation_hits,clock,capture_next,cache_bytes;
    unsigned buckets[HD_BUCKETS],width_heads[321],max_height[321];
    unsigned render_heads[HD_ASSETS],render_next[64000],render_ids[64000];
    uint32_t tile_tags[256];uint8_t tile_indices[256];
    unsigned tile_address;int tile_valid;
    uint32_t tags[4][65536],latches[4];
    Capture captures[HD_CAPTURES];
    uint8_t low[64000],scratch[64000];
    uint32_t output[1024000];
    char directory[1024];
    uint8_t previous_palette[768];
    unsigned revision,rendered_revision,rendered_start;
    int rendered;
    unsigned dialogue_x,dialogue_y,dialogue_address,dialogue_shaded;
    int dialogue_ready,dialogue_active;
    uint32_t dialogue_background[1040*320],dialogue_panel[520*160];
    uint8_t dialogue_base[260*80];
    unsigned panel_width,panel_height,panel_capture_width;
    int panel_is_menu;
    unsigned ui_depth;
    int ui_plain;
    uint32_t ui_background[1280*800],ui_panel[640*400];
};
static const uint32_t *render_page(JxHd *,const JxVideoModel *,const uint8_t *,unsigned);
static uint32_t hash_bytes(const uint8_t *p,unsigned n){
    uint32_t h=2166136261u;for(unsigned i=0;i<n;i++){h^=p[i];h*=16777619u;}return h;
}
static unsigned asset_bucket(uint32_t hash,unsigned w,unsigned height){return (hash^(w*257u)^(height*65537u))&(HD_BUCKETS-1);}
static unsigned find_asset(JxHd *h,const uint8_t *pixels,unsigned w,unsigned height){
    unsigned result=0;uint32_t hash=hash_bytes(pixels,w*height);
    for(unsigned link=h->buckets[asset_bucket(hash,w,height)];link;link=h->assets[link-1].next_hash){
        Asset *a=&h->assets[link-1];
        if(a->width==w && a->height==height && !a->failed && a->hash==hash && !memcmp(a->indices,pixels,w*height))
            if(!result || link<result)result=link;
    }
    if(height<h->max_height[w])for(unsigned link=h->width_heads[w];link;link=h->assets[link-1].next_width){
        Asset *a=&h->assets[link-1];
        if(a->height>height && !a->failed && (!result || link<result) && hash_bytes(a->indices,w*height)==hash && !memcmp(a->indices,pixels,w*height))result=link;
    }
    return result;
}
static uint16_t word(const uint8_t *p){return (uint16_t)(p[0]|((unsigned)p[1]<<8));}
static uint32_t dword(const uint8_t *p){return p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static int safe_path(const char *p){return *p && *p!='/' && *p!='\\' && !strstr(p,"..") && !strchr(p,':');}
static FILE *open_asset(JxHd *h,const char *name){
    char path[1280];if(!safe_path(name))return NULL;
    int n=snprintf(path,sizeof path,"%s/%s",h->directory,name);
    return n>0 && (size_t)n<sizeof path?fopen(path,"rb"):NULL;
}
JxHd *jx_hd_open(const char *directory){
    if(!directory || strlen(directory)>=1024)return NULL;
    JxHd *h=calloc(1,sizeof *h);if(!h)return NULL;
    strcpy(h->directory,directory);char line[1024];
    for(unsigned pack=0;pack<2;pack++){
    char path[1280];snprintf(path,sizeof path,"%s/%smanifest.tsv",directory,pack?"superres/":"");
    FILE *f=fopen(path,"rb");if(!f)continue;
    FILE *packed=open_asset(h,pack?"superres/references.bin":"references.bin");long packed_size=0;
    if(packed){
        uint8_t header[12];
        if(fread(header,1,12,packed)!=12 || memcmp(header,"JXREF1\0\0",8) || fseek(packed,0,SEEK_END) || (packed_size=ftell(packed))<12){fclose(packed);packed=NULL;}
    }
    while(h->count<HD_ASSETS && fgets(line,sizeof line,f)){
        if(line[0]=='#' || line[0]=='\r' || line[0]=='\n')continue;
        char name[160],reference[240],image[240];unsigned w,hh;int consumed=0;
        if(sscanf(line,"%159[^\t]\t%u\t%u\t%239[^\t]\t%239[^\r\n]%n",name,&w,&hh,reference,image,&consumed)!=5)continue;
        (void)consumed;
        char *policy=strchr(image,'\t');
        int truecolor=0;
        if(policy){*policy++=0;if(strcmp(policy,"truecolor")!=0)continue;truecolor=1;}
        int bundled=reference[0]=='@';
        if(pack){
            char prefixed[240];
            if(strlen(reference)+9>=sizeof prefixed || strlen(image)+9>=sizeof prefixed)continue;
            if(!bundled){snprintf(prefixed,sizeof prefixed,"superres/%s",reference);strcpy(reference,prefixed);}
            snprintf(prefixed,sizeof prefixed,"superres/%s",image);strcpy(image,prefixed);
        }
        if(!w || w>320 || !hh || hh>200 || w*hh>64000 || !safe_path(image))continue;
        FILE *ref=NULL;
        if(bundled){
            char *end=NULL;unsigned long offset=strtoul(reference+1,&end,10);
            if(!packed || reference[1]<'0' || reference[1]>'9' || !end || *end || offset<12 || offset>(unsigned long)packed_size || 768u+w*hh>(unsigned long)packed_size-offset || fseek(packed,(long)offset,SEEK_SET))continue;
            ref=packed;
        }else ref=open_asset(h,reference);
        if(!ref)continue;
        Asset *a=&h->assets[h->count];a->indices=malloc(w*hh);
        if(!a->indices){if(!bundled)fclose(ref);continue;}
        int valid=fread(a->palette,1,768,ref)==768 && fread(a->indices,1,w*hh,ref)==w*hh && (bundled || fgetc(ref)==EOF);
        if(!bundled)fclose(ref);
        if(!valid){free(a->indices);a->indices=NULL;continue;}
        a->width=w;a->height=hh;a->truecolor=truecolor;a->hash=hash_bytes(a->indices,w*hh);strcpy(a->image,image);
        a->dynamic_kind=!strncmp(name,"TILE_",5)?1:!strncmp(name,"BATTLE_",7)?2:0;
        unsigned bucket=asset_bucket(a->hash,w,hh);a->next_hash=h->buckets[bucket];h->buckets[bucket]=h->count+1;
        a->next_width=h->width_heads[w];h->width_heads[w]=h->count+1;if(hh>h->max_height[w])h->max_height[w]=hh;
        h->count++;
    }
    if(packed)fclose(packed);fclose(f);
    }
    if(!h->count){jx_hd_close(h);return NULL;}return h;
}
void jx_hd_close(JxHd *h){
    if(!h)return;
    for(unsigned i=0;i<h->count;i++){free(h->assets[i].indices);free(h->assets[i].pixels);}
    for(unsigned i=0;i<HD_CAPTURES;i++){free(h->captures[i].indices);free(h->captures[i].tags);}free(h);
}
unsigned jx_hd_asset_count(const JxHd *h){return h?h->count:0;}
unsigned jx_hd_hits(const JxHd *h){return h?h->hits:0;}
unsigned jx_hd_tile_hits(const JxHd *h){return h?h->tile_hits:0;}
unsigned jx_hd_animation_hits(const JxHd *h){return h?h->animation_hits:0;}
void jx_hd_reset(JxHd *h){
    if(!h)return;h->revision++;memset(h->tags,0,sizeof h->tags);memset(h->latches,0,sizeof h->latches);
    h->dialogue_ready=h->dialogue_active=0;
    h->tile_valid=0;
    h->ui_depth=0;h->ui_plain=0;
    for(unsigned i=0;i<HD_CAPTURES;i++)h->captures[i].width=0;
}
void jx_hd_read(JxHd *h,unsigned at){if(h && at>=0xa0000 && at<0xb0000)for(unsigned p=0;p<4;p++)h->latches[p]=h->tags[p][at-0xa0000];}
void jx_hd_write(JxHd *h,const JxVideoModel *v,unsigned at){
    if(h && at>=0xa0000 && at<0xb0000){h->revision++;for(unsigned p=0;p<4;p++)
        if(v->map_mask&(1u<<p))h->tags[p][at-0xa0000]=(v->mode&1)?h->latches[p]:0;
    }
}
static int source_bytes(JxHd *h,JxEventContext *c,const uint16_t *a){
    if(!a[2] || !a[3] || a[2]>320 || a[3]>200 || (unsigned)a[2]*a[3]>64000)return 0;
    for(unsigned i=0;i<(unsigned)a[2]*a[3];i++)h->scratch[i]=c->memory[(((uint32_t)a[5]*16)+(uint16_t)(a[4]+i))&0xfffff];
    return 1;
}
static void blit_source(JxHd *h,JxEventContext *c,const JxVideoModel *v,const uint16_t *a,int transparent){
    if(h->dialogue_ready && a[0]==h->dialogue_x && a[1]==h->dialogue_y && a[2]==h->panel_capture_width && a[3]==h->panel_height && transparent<0){
        unsigned address=((unsigned)a[5]<<16)|a[4];
        if(address==h->dialogue_shaded){
            for(unsigned y=0;y<h->panel_height;y++)memcpy(h->dialogue_base+y*h->panel_width,h->scratch+y*a[2],a[2]);
            h->dialogue_active=1;h->revision++;
        }
        else if(address==h->dialogue_address){h->dialogue_active=0;h->dialogue_ready=0;h->revision++;}
    }
    unsigned w=a[2],hh=a[3],n=w*hh,page=(c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8));
    unsigned match=find_asset(h,h->scratch,w,hh),id=match?match-1:0;Asset *found=match?&h->assets[id]:NULL;const uint32_t *captured=NULL;
    if(!found)for(unsigned i=0;i<HD_CAPTURES;i++){
        Capture *cap=&h->captures[(h->capture_next+HD_CAPTURES-1-i)%HD_CAPTURES];
        if(cap->width==w && cap->height==hh && cap->address==(((uint32_t)a[5]<<16)|a[4]) && !memcmp(cap->indices,h->scratch,n)){captured=cap->tags;break;}
    }
    if(!found && !captured)return;
    h->revision++;
    for(unsigned y=0;y<hh && a[1]+y<200;y++)for(unsigned x=0;x<w && a[0]+x<320;x++){
        unsigned i=y*w+x,value=h->scratch[i],px=a[0]+x,py=a[1]+y;
        if(transparent>=0 && value==(unsigned)transparent)continue;
        unsigned at=(page+py*80+px/4)&65535;
        if(v->planes[px%4][at]==value)h->tags[px%4][at]=found?((id+1)<<16)|(i+1):captured[i];
    }
    if(found){++h->hits;if(found->dynamic_kind==2)h->animation_hits++;if(found->dynamic_kind==1)h->tile_hits++;}
}
void jx_hd_blit(JxHd *h,JxEventContext *c,const JxVideoModel *v,const uint16_t *a,int transparent){
    if(!h || !source_bytes(h,c,a))return;
    blit_source(h,c,v,a,transparent);
}
static unsigned far_address(unsigned segment,unsigned offset){return (segment*16+offset)&0xfffff;}
static unsigned tile_offset(unsigned p){return (p%16%4)*64+(p/16)*4+(p%16)/4;}
void jx_hd_tile_copy(JxHd *h,JxEventContext *c,const uint16_t *a,int transparent){
    if(!h)return;
    unsigned dest=far_address(a[1],a[0]);
    if(dest!=far_address(jx_word(c,0x2eb2),jx_word(c,0x2eb0)))return;
    if(a[4]!=256){h->tile_valid=0;return;}
    unsigned source=far_address(a[3],a[2]);
    if(source<dest+256 && dest<source+256){h->tile_valid=0;return;}
    if(!h->tile_valid || h->tile_address!=dest){
        memset(h->tile_tags,0,sizeof h->tile_tags);
        for(unsigned p=0;p<256;p++)h->tile_indices[p]=c->memory[far_address(a[1],(uint16_t)(a[0]+tile_offset(p)))];
    }
    for(unsigned p=0;p<256;p++)h->scratch[p]=c->memory[far_address(a[3],(uint16_t)(a[2]+tile_offset(p)))];
    unsigned match=find_asset(h,h->scratch,16,16);int changed=0;
    for(unsigned p=0;p<256;p++){
        unsigned value=h->scratch[p];if(transparent>=0 && value==(unsigned)transparent)continue;
        h->tile_indices[p]=(uint8_t)value;h->tile_tags[p]=match?(match<<16)|(p+1):0;changed=1;
    }
    if(match && changed){h->hits++;if(h->assets[match-1].dynamic_kind==1)h->tile_hits++;}
    h->tile_address=dest;h->tile_valid=1;
}
void jx_hd_tile_capture(JxHd *h,JxEventContext *c,const JxVideoModel *v,const uint16_t *a){
    if(!h || a[2]!=16 || a[3]!=16 || a[0]>304 || a[1]>184)return;
    unsigned dest=far_address(a[5],a[4]);
    if(dest!=far_address(jx_word(c,0x2eb2),jx_word(c,0x2eb0)))return;
    unsigned page=c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8);
    for(unsigned p=0;p<256;p++){
        unsigned x=a[0]+p%16,y=a[1]+p/16,at=(page+y*80+x/4)&65535;
        h->tile_indices[p]=v->planes[x%4][at];h->tile_tags[p]=h->tags[x%4][at];
    }
    h->tile_address=dest;h->tile_valid=1;
}
void jx_hd_blit_planes(JxHd *h,JxEventContext *c,const JxVideoModel *v,const uint16_t *a){
    if(!h)return;
    uint16_t adjusted[6];memcpy(adjusted,a,sizeof adjusted);adjusted[2]&=(uint16_t)~7u;
    unsigned w=adjusted[2],hh=adjusted[3];
    if(!w || w>320 || !hh || hh>200 || w*hh>64000)return;
    for(unsigned y=0;y<hh;y++)for(unsigned x=0;x<w;x++){
        unsigned offset=(x%4)*(w/4)*hh+y*(w/4)+x/4;
        h->scratch[y*w+x]=c->memory[(((uint32_t)a[5]*16)+(uint16_t)(a[4]+offset))&0xfffff];
    }
    if(w==16 && hh==16 && h->tile_valid && h->tile_address==far_address(a[5],a[4]) && !memcmp(h->tile_indices,h->scratch,256)){
        unsigned page=c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8);h->revision++;
        for(unsigned p=0;p<256;p++){
            unsigned x=a[0]+p%16,y=a[1]+p/16;if(x>=320 || y>=200)continue;
            unsigned at=(page+y*80+x/4)&65535;
            if(v->planes[x%4][at]==h->scratch[p])h->tags[x%4][at]=h->tile_tags[p];
        }
        h->tile_valid=0;
    }else {h->tile_valid=0;blit_source(h,c,v,adjusted,-1);}
}
void jx_hd_capture(JxHd *h,JxEventContext *c,const uint16_t *a){
    if(!h || !source_bytes(h,c,a))return;
    Capture *cap=&h->captures[h->capture_next++%HD_CAPTURES];unsigned n=(unsigned)a[2]*a[3];
    free(cap->indices);free(cap->tags);cap->width=0;
    cap->indices=malloc(n);cap->tags=calloc(n,sizeof(uint32_t));if(!cap->indices || !cap->tags)return;
    memcpy(cap->indices,h->scratch,n);cap->width=a[2];cap->height=a[3];cap->address=((uint32_t)a[5]<<16)|a[4];
    unsigned page=c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8);
    for(unsigned y=0;y<a[3] && a[1]+y<200;y++)for(unsigned x=0;x<a[2] && a[0]+x<320;x++){
        unsigned px=a[0]+x,py=a[1]+y;cap->tags[y*a[2]+x]=h->tags[px%4][(page+py*80+px/4)&65535];
    }
}
void jx_hd_dialogue_capture(JxHd *h,JxEventContext *c,const JxVideoModel *v,const uint8_t *palette,const uint16_t *a){
    int dialogue=a[2]==260 && a[3]==80;
    int menu=(a[2]==72 && a[3]>=20 && a[3]<=100 && a[3]%20==0) || (a[2]==100 && a[3]==60);
    /* Inventory grant draws text directly after capture, without shading. */
    int notice=a[0]==100 && a[1]==100 && a[2]==160 && a[3]==17;
    unsigned width=menu && a[2]==72?84:a[2];
    if(menu && a[2]==72 && a[0]<320 && a[0]+width>320)width=320-a[0];
    if(!h || h->ui_depth || (!dialogue && !menu && !notice) || a[0]+width>320 || a[1]+a[3]>200 ||
       a[4]!=jx_word(c,0x2eb0) || a[5]!=jx_word(c,0x2eb2))return;
    h->dialogue_active=0;h->revision++;
    unsigned page=c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8);
    const uint32_t *pixels=render_page(h,v,palette,page);
    for(unsigned y=0;y<a[3]*4u;y++)memcpy(h->dialogue_background+y*width*4,pixels+(a[1]*4+y)*1280+a[0]*4,width*16);
    for(unsigned y=0;y<a[3];y++)for(unsigned x=0;x<width;x++)
        h->dialogue_base[y*width+x]=v->planes[(a[0]+x)%4][(page+(a[1]+y)*80+(a[0]+x)/4)&65535];
    h->panel_width=width;h->panel_height=a[3];h->panel_capture_width=a[2];h->panel_is_menu=menu || notice;
    h->dialogue_x=a[0];h->dialogue_y=a[1];h->dialogue_address=((unsigned)a[5]<<16)|a[4];
    uint32_t sum=(uint32_t)a[4]+(dialogue?0x7d00:(unsigned)a[2]*a[3]);
    h->dialogue_shaded=((uint32_t)(uint16_t)(a[5]+(sum>>16)*0x1000)<<16)|(uint16_t)sum;
    h->dialogue_ready=1;
    if(notice){h->dialogue_active=1;h->revision++;}
}
void jx_hd_ui_begin(JxHd *h,const JxVideoModel *v,const uint8_t *palette,int plain){
    if(!h)return;
    if(!h->ui_depth){
        const uint32_t *pixels=jx_hd_render(h,v,palette);
        memcpy(h->ui_background,pixels,sizeof h->ui_background);
        h->ui_plain=plain;
    }
    h->ui_depth++;h->revision++;
}
void jx_hd_ui_end(JxHd *h){
    if(h && h->ui_depth){h->ui_depth--;if(!h->ui_depth)h->ui_plain=0;h->revision++;}
}
static void scale2(const uint8_t *src,unsigned w,unsigned h,uint8_t *out){
    for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++){
        unsigned i=y*w+x;uint8_t e=src[i],b=src[(y?y-1:y)*w+x],d=src[y*w+(x?x-1:x)],
            f=src[y*w+(x+1<w?x+1:x)],g=src[(y+1<h?y+1:y)*w+x];
        unsigned p=y*2*w*2+x*2;
        out[p]=(d==b && b!=f && d!=g)?d:e;out[p+1]=(b==f && b!=d && f!=g)?f:e;
        out[p+w*2]=(d==g && d!=b && g!=f)?d:e;out[p+w*2+1]=(g==f && d!=g && b!=f)?f:e;
    }
}
void jx_hd_scale4(const uint8_t *src,unsigned w,unsigned h,uint8_t *twice,uint8_t *out){scale2(src,w,h,twice);scale2(twice,w*2,h*2,out);}
static uint32_t *load_pixels(JxHd *h,Asset *a){
    if(a->pixels){a->used=++h->clock;return a->pixels;}if(a->failed)return NULL;
    FILE *f=open_asset(h,a->image);uint8_t head[54];
    unsigned w=a->width*4,hh=a->height*4;
    if(!f)return a->failed=1,(uint32_t *)NULL;
    int valid=fread(head,1,54,f)==54 && head[0]=='B' && head[1]=='M' && dword(head+14)==40 &&
        dword(head+18)==w && (dword(head+22)==hh || dword(head+22)==(uint32_t)(-(int)hh)) &&
        word(head+26)==1 && word(head+28)==32 && dword(head+30)==0 && dword(head+10)>=54;
    if(!valid || dword(head+10)>1048576 || fseek(f,(long)dword(head+10),SEEK_SET)) {fclose(f);a->failed=1;return NULL;}
    while(h->cache_bytes+w*hh*4>HD_CACHE_BYTES){
        unsigned oldest=h->count;
        for(unsigned i=0;i<h->count;i++)if(h->assets[i].pixels && (oldest==h->count || h->assets[i].used<h->assets[oldest].used))oldest=i;
        if(oldest==h->count)break;
        Asset *evicted=&h->assets[oldest];h->cache_bytes-=evicted->width*evicted->height*64;
        free(evicted->pixels);evicted->pixels=NULL;
    }
    a->pixels=malloc(w*hh*4);if(!a->pixels){fclose(f);return NULL;}
    int top=(int32_t)dword(head+22)<0;
    for(unsigned y=0;y<hh;y++)if(fread(a->pixels+(top?y:hh-1-y)*w,4,w,f)!=w){free(a->pixels);a->pixels=NULL;a->failed=1;break;}
    fclose(f);if(a->pixels)h->cache_bytes+=w*hh*4;a->used=++h->clock;return a->pixels;
}
static uint32_t palette_color(const uint8_t *p,unsigned i){return ((uint32_t)p[i*3]*4<<16)|((uint32_t)p[i*3+1]*4<<8)|((uint32_t)p[i*3+2]*4);}
static const uint32_t *render_page(JxHd *h,const JxVideoModel *v,const uint8_t *palette,unsigned start){
    if(!h)return NULL;
    int changed=!h->rendered || h->rendered_revision!=h->revision || h->rendered_start!=start || memcmp(palette,h->previous_palette,768);
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++){
        uint8_t index=v->planes[x%4][(start+y*80+x/4)&65535];
        if(h->low[y*320+x]!=index)changed=1;h->low[y*320+x]=index;
    }
    if(!changed)return h->output;
    memcpy(h->previous_palette,palette,768);h->rendered_revision=h->revision;h->rendered_start=start;h->rendered=1;
    uint32_t colors[256];for(unsigned i=0;i<256;i++)colors[i]=palette_color(palette,i);
    /* Unknown pixels include live font strokes. Preserve every original bit;
     * edge-aware scaling here reshaped both Chinese and ASCII glyphs. */
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++){
        uint32_t color=colors[h->low[y*320+x]];
        for(unsigned dy=0;dy<4;dy++)for(unsigned dx=0;dx<4;dx++)h->output[(y*4+dy)*1280+x*4+dx]=color;
    }
    /* Cache pointers per frame so LRU eviction never happens mid-composition. */
    for(unsigned i=0;i<h->count;i++)h->render_heads[i]=0;
    unsigned needed_count=0;
    for(unsigned y=0;!h->ui_plain && y<200;y++)for(unsigned x=0;x<320;x++){
        uint32_t tag=h->tags[x%4][(start+y*80+x/4)&65535];unsigned id=(tag>>16)-1;
        if(tag && id<h->count){
            unsigned pos=y*320+x;if(!h->render_heads[id])h->render_ids[needed_count++]=id;
            h->render_next[pos]=h->render_heads[id];h->render_heads[id]=pos+1;
        }
    }
    for(unsigned item=0;item<needed_count;item++){
        unsigned id=h->render_ids[item];
        Asset *a=&h->assets[id];uint32_t *pixels=load_pixels(h,a);if(!pixels)continue;
        unsigned factors[256][3];
        unsigned fade=256;
        if(a->truecolor){
            uint64_t original=0,current=0;
            for(unsigned p=0;p<a->width*a->height;p++){
                unsigned index=a->indices[p];if(index==255)continue;
                for(unsigned ch=0;ch<3;ch++){original+=a->palette[index*3+ch];current+=palette[index*3+ch];}
            }
            if(original){uint64_t ratio=current*256/original;fade=ratio>256?256:(unsigned)ratio;}
            else {for(unsigned i=0;i<768;i++)current+=palette[i];fade=current?256:0;}
        }
        for(unsigned i=0;i<256;i++)for(unsigned ch=0;ch<3;ch++){
            unsigned base=a->palette[i*3+ch],now=palette[i*3+ch];
            unsigned sum=a->palette[i*3]+a->palette[i*3+1]+a->palette[i*3+2];
            unsigned current=palette[i*3]+palette[i*3+1]+palette[i*3+2];
            factors[i][ch]=a->truecolor?fade:base?now*256/base:sum?current*256/sum:now?256:0;
        }
        for(unsigned link=h->render_heads[id];link;link=h->render_next[link-1]){
            unsigned x=(link-1)%320,y=(link-1)/320;
            uint32_t tag=h->tags[x%4][(start+y*80+x/4)&65535];
            if((tag>>16)!=id+1 || !(tag&65535))continue;
            unsigned p=(tag&65535)-1;if(p>=a->width*a->height)continue;
            unsigned index=a->indices[p];if(h->low[y*320+x]!=index)continue;
            unsigned sx=p%a->width*4,sy=p/a->width*4;
            int unchanged=factors[index][0]==256 && factors[index][1]==256 && factors[index][2]==256;
            for(unsigned dy=0;dy<4;dy++)for(unsigned dx=0;dx<4;dx++){
                uint32_t rgb=pixels[(sy+dy)*a->width*4+sx+dx],out=0;
                if(unchanged){h->output[(y*4+dy)*1280+x*4+dx]=rgb&0xffffff;continue;}
                for(unsigned ch=0;ch<3;ch++){unsigned shift=16-ch*8,value=((rgb>>shift)&255)*factors[index][ch]/256;if(value>255)value=255;out|=value<<shift;}
                h->output[(y*4+dy)*1280+x*4+dx]=out;
            }
        }
    }
    if(h->dialogue_active && !h->ui_depth){
        unsigned left=h->dialogue_x*4,top=h->dialogue_y*4;
        unsigned width=h->panel_width,height=h->panel_height,pw=width*2,ph=height*2;
        /* Only text/border changes over the original shaded rectangle belong
         * to the panel. Its sampled background must never be moved/scaled. */
        for(unsigned y=0;y<ph;y++)for(unsigned x=0;x<pw;x++){
            unsigned sx=x/2,sy=y/2;
            int changed=h->low[(h->dialogue_y+sy)*320+h->dialogue_x+sx]!=h->dialogue_base[sy*width+sx];
            h->dialogue_panel[y*pw+x]=changed?0x80000000u|h->output[(top+y*2)*1280+left+x*2]:0;
        }
        for(unsigned y=0;y<height*4;y++)memcpy(h->output+(top+y)*1280+left,h->dialogue_background+y*width*4,width*16);
        unsigned offset=h->panel_is_menu?0:h->dialogue_y==0?0:h->dialogue_y==112?520:260;
        unsigned yoffset=h->panel_is_menu?0:80;
        for(unsigned y=0;y<ph;y++)for(unsigned x=0;x<pw;x++){
            uint32_t pixel=h->dialogue_panel[y*pw+x];
            if(pixel&0x80000000u)h->output[(top+yoffset+y)*1280+left+offset+x]=pixel&0xffffff;
        }
    }
    if(h->ui_depth){
        for(unsigned y=0;y<400;y++)for(unsigned x=0;x<640;x++)h->ui_panel[y*640+x]=h->output[y*2*1280+x*2];
        memcpy(h->output,h->ui_background,sizeof h->output);
        for(unsigned y=0;y<400;y++)memcpy(h->output+(200+y)*1280+320,h->ui_panel+y*640,640*4);
    }
    return h->output;
}
const uint32_t *jx_hd_render(JxHd *h,const JxVideoModel *v,const uint8_t *palette){
    return render_page(h,v,palette,(unsigned)v->start_high<<8);
}
int jx_hd_export_bmp(JxHd *h,const JxVideoModel *v,const uint8_t *palette,const char *path){
    const uint32_t *pixels=jx_hd_render(h,v,palette);if(!pixels)return 0;
    FILE *f=fopen(path,"wb");if(!f)return 0;
    uint8_t header[54]={0};header[0]='B';header[1]='M';
    const unsigned offsets[]={2,10,14,18,22,26,28,34};
    const uint32_t values[]={54+1280*800*4,54,40,1280,(uint32_t)-800,1,32,1280*800*4};
    for(unsigned i=0;i<8;i++)for(unsigned b=0;b<((i==5 || i==6)?2u:4u);b++)header[offsets[i]+b]=(uint8_t)(values[i]>>(b*8));
    int good=fwrite(header,1,54,f)==54 && fwrite(pixels,4,1280*800,f)==1280*800;
    if(fclose(f))good=0;return good;
}
