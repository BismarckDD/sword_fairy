#include "event_engine.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t memory[1048576];
static JxEventContext context;
static const uint16_t (*events)[2];static unsigned count,index;
static unsigned interactions,saves,loads,exits,captures;static uint16_t load_result;
void jx_save_game_menu(JxEventContext *c,JxFarPointer work){(void)c;(void)work;++saves;}
uint16_t jx_load_game_menu(JxEventContext *c,JxFarPointer work){(void)c;(void)work;++loads;return load_result;}
static int browser(void *opaque,int mode){(void)opaque;if(mode==2)++captures;return -2;}
static uint32_t dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    (void)opaque;
    if(segment!=0x1f2a)return 0;
    if(offset==0xc1f2){assert(n==2 && index<count);JxFarPointer p={a[0],a[1]};jx_far_set_byte(&context,p,0,(uint8_t)events[index][0]);jx_far_set_byte(&context,p,1,0);jx_far_set_byte(&context,p,2,(uint8_t)events[index][1]);jx_far_set_byte(&context,p,3,0);index++;}
    if(offset==0x7606)++interactions;
    if(offset==0xb196)++exits;
    return 0;
}
static void run(const uint16_t (*script)[2],unsigned length) {
    events=script;count=length;index=0;
    memset(memory+0x96000+142,0xa5,32);
    assert(jx_game_loop_step(&context,(JxFarPointer){0,0x9600})==JX_GAME_LOOP_PROCESSED);
    assert(index==count);for(unsigned i=0;i<32;i++)assert(memory[0x96000+142+i]==0xa5);
}
int main(void) {
    context=(JxEventContext){memory,0x39e3,dispatch,NULL,browser,NULL};
    static const uint16_t enter[][2]={{0,0x1c}},space[][2]={{0,0x39}};
    run(enter,1);run(space,1);assert(interactions==2 && captures==0);
    static const uint16_t save_cancel_load[][2]={{1,1},{1,0x50},{1,0x50},{1,0x50},{0,0x39},{1,0x50},{1,0x50},{1,0x50},{0,0x1c},{1,0x48},{0,0x39},{1,1},{1,1}};
    load_result=1;run(save_cancel_load,sizeof save_cancel_load/sizeof save_cancel_load[0]);assert(saves==1 && loads==1 && captures==1);
    static const uint16_t load_success[][2]={{1,1},{1,0x50},{1,0x50},{1,0x50},{0,0x1c},{1,0x50},{1,0x50},{0,0x39}};
    load_result=0;run(load_success,sizeof load_success/sizeof load_success[0]);assert(loads==2);
    static const uint16_t cancel_exit[][2]={{1,1},{1,0x50},{1,0x50},{1,0x50},{0,0x1c},{1,0x48},{0,0x39},{0,0x1c},{1,1},{1,1}};
    run(cancel_exit,sizeof cancel_exit/sizeof cancel_exit[0]);assert(exits==0);
    static const uint16_t confirm_exit[][2]={{1,1},{1,0x50},{1,0x50},{1,0x50},{0,0x39},{1,0x48},{0,0x1c},{1,0x48},{0,0x39},{1,1},{1,1}};
    run(confirm_exit,sizeof confirm_exit/sizeof confirm_exit[0]);assert(exits==1);
    puts("PASS: Enter/Space world interaction, save/load cancellation return, successful load closes menus, exit default cancel and explicit confirm, original 142-byte frame bounds.");return 0;
}
