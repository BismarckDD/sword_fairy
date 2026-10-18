#include "native_hd.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t memory[1048576],palette[768],tiles[240][256];
static JxVideoModel video;
static unsigned plane_offset(unsigned p){return (p%16%4)*64+(p/16)*4+(p%16)/4;}
static double seconds(void){LARGE_INTEGER t,f;QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return (double)t.QuadPart/(double)f.QuadPart;}
int main(int argc,char **argv){
    unsigned rounds=argc>1?(unsigned)strtoul(argv[1],NULL,10):24;
    FILE *manifest=fopen("runtime/game/hd/superres/manifest.tsv","rb");if(!manifest)return 2;
    char line[1024];unsigned count=0;
    while(count<240 && fgets(line,sizeof line,manifest)){
        char key[160],ref[240],image[240];unsigned w,h;
        if(line[0]=='#' || sscanf(line,"%159[^\t]\t%u\t%u\t%239[^\t]\t%239[^\r\n]",key,&w,&h,ref,image)!=5 || w!=16 || h!=16)continue;
        char path[512];snprintf(path,sizeof path,"runtime/game/hd/superres/references/%s.idx",key);
        FILE *f=fopen(path,"rb");if(!f)return 3;
        if(fread(palette,1,768,f)!=768 || fread(tiles[count],1,256,f)!=256)return 4;fclose(f);count++;
    }
    fclose(manifest);if(count!=240)return 5;
    double begin=seconds();JxHd *hd=jx_hd_open("runtime/game/hd");if(!hd)return 6;double opened=seconds()-begin;
    JxEventContext c={memory,0,NULL,NULL,NULL,NULL};jx_set_word(&c,0x2eb0,0);jx_set_word(&c,0x2eb2,0x8000);
    uint16_t copy[]={0,0x8000,0,0x8200,256};unsigned before=jx_hd_hits(hd);
    begin=seconds();
    for(unsigned frame=0;frame<rounds;frame++)for(unsigned t=0;t<240;t++){
        for(unsigned p=0;p<256;p++)memory[0x82000+plane_offset(p)]=tiles[t][p];
        jx_hd_tile_copy(hd,&c,copy,-1);
    }
    double lookup=seconds()-begin;
    begin=seconds();
    for(unsigned frame=0;frame<9;frame++){
        if(frame==1)begin=seconds(); /* Exclude initial BMP reads from the frame comparison. */
        for(unsigned t=0;t<240;t++){
            unsigned which=(t+frame)%240;
            for(unsigned p=0;p<256;p++)memory[0x82000+plane_offset(p)]=tiles[which][p];
            jx_hd_tile_copy(hd,&c,copy,-1);memcpy(memory+0x80000,memory+0x82000,256);
            uint16_t draw[]={(uint16_t)((t%20)*16),(uint16_t)((t/20)*16),16,16,0,0x8000};
            for(unsigned p=0;p<256;p++){
                unsigned x=draw[0]+p%16,y=draw[1]+p/16;
                video.planes[x%4][y*80+x/4]=tiles[which][p];
            }
            jx_hd_blit_planes(hd,&c,&video,draw);
        }
        if(!jx_hd_render(hd,&video,palette))return 7;
    }
    double frame_time=(seconds()-begin)/8;
    printf("{\"assets\":%u,\"draws\":%u,\"open_seconds\":%.6f,\"lookup_ms_per_240_tiles\":%.4f,\"compose_ms_per_frame\":%.4f,\"matched\":%u}\n",
        jx_hd_asset_count(hd),rounds*240,opened,lookup*1000/rounds,frame_time*1000,jx_hd_hits(hd)-before);
    jx_hd_close(hd);return 0;
}
