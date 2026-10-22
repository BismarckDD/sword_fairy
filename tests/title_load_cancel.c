#include "event_engine.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t memory[1048576];
static unsigned titles,backgrounds,loads,new_games,maps,ready;
static unsigned title_redraws;
static int load_after_cancel;
static uint32_t dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    (void)opaque;(void)count;
    if(segment==0x2f28 && offset==0x6dfe) {
        assert(args[0]==0x043b && loads==1 && titles==1);
        title_redraws++;
    }
    if(segment!=0x1f2a)return 0;
    if(offset==0x58fa) {
        if(args[0]==0x04ca)backgrounds++;
        if(args[0]==0x04e2)ready++;
    }
    if(offset==0x7baa) {
        assert(backgrounds==titles+1);
        assert(title_redraws==titles);
        assert(++titles<=3);
        return titles==1 || load_after_cancel?1:0;
    }
    if(offset==0xa2f0){assert(++loads<=2);return loads==1?1:0;}
    if(offset==0x629e)new_games++;
    if(offset==0x6700)maps++;
    return 0;
}
int main(void) {
    JxEventContext c={memory,0x39e3,dispatch,NULL,NULL,NULL};
    for(int scenario=0;scenario<2;scenario++) {
        memset(memory,0,sizeof memory);
        titles=backgrounds=loads=new_games=maps=ready=0;
        title_redraws=0;
        load_after_cancel=scenario;
        jx_game_initialize(&c);
        assert(titles==2 && backgrounds==2 && ready==1);
        assert(title_redraws==1);
        assert(loads==(scenario?2u:1u));
        assert(new_games==(scenario?0u:1u));
        assert(maps==(scenario?0u:1u));
    }
    puts("title load cancellation PASS: return to title, new story and retry load");
    return 0;
}
