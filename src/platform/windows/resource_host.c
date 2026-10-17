/* Native integration probe. Original resource files and recovered C only.
 * FILE/EMS/DOS are host boundaries; unknown game calls are fatal.
 * This probe is not yet the complete interactive game frontend. */
#include "event_engine.h"
#include "video_model.h"
#include "native_window.h"
#include "native_audio.h"
#include "native_cd.h"
#include "walkthrough_route.h"
#include "temple_walkthrough_route.h"
#include "hall_walkthrough_route.h"
#include "disciple_walkthrough_route.h"
#include "disciple_second_route.h"
#include "disciple_third_route.h"
#include "return_walkthrough_route.h"
#include "blood_walkthrough_route.h"
#include "east_walkthrough_route.h"
#include "road_walkthrough_route.h"
#include "wuyi_walkthrough_route.h"
#include "liu_house_route.h"
#include "zhang_lin_route.h"
#include "liu_return_route.h"
#include "wuyi_return_route.h"
#include "linan_route.h"
#include "linan_gate_route.h"
#include "city_information_routes.h"
#include "brothel_route.h"
#include "tavern_route.h"
#include "tavern_clue_route.h"
#include "tavern_upstairs_route.h"
#include "tavern_story_route.h"
#include "tavern_downstairs_route.h"
#include "tavern_departure_route.h"
#include "casino_route.h"
#include "casino_token_route.h"
#include "casino_return_route.h"
#include "token_guard_route.h"
#include "xiao_lei_route.h"
#include "fishing_village_route.h"
#include "fisherman_clue_route.h"
#include "old_wang_house_route.h"
#include "old_wang_clue_route.h"
#include "old_wang_return_route.h"
#include "village_return_route.h"
#include "jia_clue_route.h"
#include "nangong_entry_route.h"
#include "nangong_cup_route.h"
#include "brothel_downstairs_route.h"
#include "brothel_city_return_route.h"
#include "jia_cup_route.h"
#include "sailing_village_route.h"
#include "sailing_house_route.h"
#include "sailing_wang_route.h"
#include "sailing_pier_return_route.h"
#include "island_sailing_route.h"
#include "island_killer_route.h"
#include "island_killer_wu_route.h"
#include "island_killer_bing_route.h"
#include "island_killer_yi_route.h"
#include "island_killer_jia_route.h"
#include "island_clearance_route.h"
#include "island_hideout_route.h"
#include "island_zhang_route.h"
#include "island_door_route.h"
#include "island_cellar_route.h"
#include "island_he_mei_route.h"
#include "island_cellar_return_route.h"
#include "island_hideout_return_route.h"
#include "island_sailing_return_route.h"
#include "village_reunion_route.h"
#include "rescued_city_return_route.h"
#include "he_mei_house_route.h"
#include "he_mei_proposal_route.h"
#include "he_mei_gift_route.h"
#include "he_mei_departure_route.h"
#include "city_wall_entry_route.h"
#include "city_wall_zhang_route.h"
#include "inn_downstairs_route.h"
#include "inn_owner_release_route.h"
#include "inn_city_return_route.h"
#include "city_wall_return_route.h"
#include "city_wall_reunion_route.h"
#include "city_wall_departure_route.h"
#include "linan_wuyi_departure_route.h"
#include "wuyi_second_entrance_route.h"
#include "wuyi_second_disciple_route.h"
#include "wuyi_killer_first_route.h"
#include "wuyi_killer_second_route.h"
#include "wuyi_killer_third_route.h"
#include "wuyi_killer_fourth_route.h"
#include "wuyi_killer_fifth_route.h"
#include "wuyi_second_liu_house_route.h"
#include "liu_wounded_clue_route.h"
#include "liu_wounded_departure_route.h"
#include "wuyi_rear_cave_entry_route.h"
#include "wuyi_inner_cave_entry_route.h"
#include "wuyi_cave_key_first_route.h"
#include "wuyi_cave_key_second_route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>
#include <direct.h>

typedef struct {
    JxEventContext c;
    uint8_t memory[1048576], banks[256][16384], palette[768];
    int mapping[4];
    uint16_t ems_frame;
    uint16_t heap_owner[0x4000];
    uint16_t heap_size[0x4000];
    FILE *files[128];
    uint8_t writable[128];
    unsigned next_file, dac, calls, maps;
    JxVideoStartupModel startup;
    JxLegacyVgaModel legacy_vga;
    JxVideoModel *video;
    JxVideoBus raw,device;
    JxNativeWindow window;
    JxNativeAudio audio;
    JxNativeCd cd;
    unsigned sound_enabled,audio_probe,audio_exports,window_probe,audio_irqs;
    uint8_t audio_irq_pending;
    uint8_t dsp_queue[4],dsp_cursor,dsp_count,dsp_arguments,pic[2],dma_ports[256];
    uint16_t trace_segment[32],trace_offset[32];unsigned trace_position;
    JxPortDevice ports;
    uint32_t vectors[256];
    uint8_t scan_code,keyboard_control;
    unsigned cd_requests;
    JxEngineCall input_fallback;
    void *input_fallback_opaque;
    unsigned menu_probe,input_probe_step,adaptive_resume_slot,adaptive_death_loaded;
    const uint16_t (*ui_events)[2];unsigned ui_count,ui_index;const char *ui_name;
    unsigned test_supply_stage,test_supply_edits;
    uint16_t contact_seg,contact_off;
    unsigned contact_hits;
    unsigned contact_walk;
    uint16_t contact_expected_x,contact_expected_y;
    unsigned scheduled_contact;
    uint8_t contact_release;
    unsigned story_probe,story_poll_steps,battle_input,opening_depth;
    unsigned boot_probe,boot_seed,portable_media,scripted_battles;
    unsigned original_init_probe,original_entry_probe,entry_world,entry_ticks;
    unsigned walkthrough_probe,walkthrough_phase,walkthrough_waypoint,temple_walkthrough,hall_walkthrough,disciple_walkthrough,return_walkthrough,blood_walkthrough;
    unsigned walkthrough_battles,walkthrough_battle_result,east_walkthrough,road_walkthrough,wuyi_walkthrough,liu_house_walkthrough,zhang_lin_walkthrough,liu_return_walkthrough;
    unsigned walkthrough_last_x,walkthrough_last_y,walkthrough_last_map,walkthrough_last_phase,walkthrough_last_progress_tick;
    unsigned wuyi_return_walkthrough,linan_walkthrough,linan_gate_walkthrough,city_information_walkthrough,city_information_index;
    unsigned brothel_walkthrough,tavern_walkthrough,tavern_clue_walkthrough,tavern_upstairs_walkthrough;
    unsigned tavern_story_walkthrough,tavern_story_battles,tavern_story_battle_result,tavern_story_contacts;
    unsigned tavern_downstairs_walkthrough;
    unsigned tavern_departure_walkthrough;
    unsigned casino_walkthrough,casino_return_walkthrough,token_guard_walkthrough,xiao_lei_walkthrough,fishing_village_walkthrough,fisherman_clue_walkthrough,old_wang_house_walkthrough,old_wang_clue_walkthrough,old_wang_return_walkthrough,village_return_walkthrough,jia_clue_walkthrough,nangong_entry_walkthrough,nangong_cup_walkthrough,brothel_downstairs_walkthrough,brothel_city_return_walkthrough,jia_cup_walkthrough,sailing_village_walkthrough,sailing_house_walkthrough,sailing_wang_walkthrough,sailing_pier_return_walkthrough,island_sailing_walkthrough,island_killer_walkthrough,island_killer_wu_walkthrough,island_killer_bing_walkthrough,island_killer_yi_walkthrough,island_killer_jia_walkthrough,island_clearance_walkthrough,island_hideout_walkthrough,island_rescue_walkthrough,island_zhang_walkthrough,island_door_walkthrough,island_cellar_walkthrough,island_he_mei_walkthrough,island_cellar_return_walkthrough,island_hideout_return_walkthrough,island_sailing_return_walkthrough,village_reunion_walkthrough,rescued_city_return_walkthrough,he_mei_house_walkthrough,he_mei_proposal_walkthrough,he_mei_gift_walkthrough,he_mei_departure_walkthrough,city_wall_entry_walkthrough,city_wall_zhang_walkthrough,inn_downstairs_walkthrough,inn_owner_release_walkthrough,inn_city_return_walkthrough,city_wall_return_walkthrough,city_wall_reunion_walkthrough,city_wall_departure_walkthrough,linan_wuyi_departure_walkthrough,wuyi_second_entrance_walkthrough,wuyi_second_disciple_walkthrough,wuyi_killer_first_walkthrough,wuyi_killer_second_walkthrough,wuyi_killer_third_walkthrough,wuyi_killer_fourth_walkthrough,wuyi_killer_fifth_walkthrough,wuyi_second_liu_house_walkthrough,liu_wounded_clue_walkthrough,liu_wounded_departure_walkthrough,wuyi_rear_cave_entry_walkthrough,wuyi_inner_cave_entry_walkthrough,wuyi_cave_key_first_walkthrough,wuyi_cave_key_second_walkthrough;
    /* Host-only route control. Commands can read stop conditions and send keys;
     * no command permits writing guest coordinates, flags, inventory or stats. */
    unsigned saved_walkthrough;
    unsigned adaptive_walkthrough,adaptive_ready,adaptive_expected_map,adaptive_goal_map;
    unsigned adaptive_watch_offset,adaptive_watch_value,adaptive_goal_x,adaptive_goal_y;
    unsigned adaptive_end_facing,adaptive_end_action,adaptive_skill,adaptive_choice,adaptive_count;
    unsigned adaptive_diagnostic_calls;
    unsigned adaptive_map81_calls;
    unsigned adaptive_skill_row,adaptive_skill_column;
    uint16_t adaptive_route[4096][2];
    char adaptive_name[64];
    unsigned island_rescue_battles,island_rescue_result;
    unsigned island_killer_battles,island_killer_result;
    unsigned jia_cup_callbacks,jia_cup_presentations;
    unsigned token_inventory_active,token_inventory_polls,token_inventory_column,token_inventory_row,token_inventory_scroll,token_inventory_scan,token_guard_callbacks;
    unsigned token_inventory_target;
    unsigned casino_token_walkthrough,casino_token_battles,casino_token_battle_result,casino_token_gambles,casino_token_gamble_result;
    unsigned walkthrough_nested_empty_polls;
    uint8_t walkthrough_scan;
    unsigned interactive_entry,interactive_limit;
    uint16_t entry_start;
    uint32_t probe_clock_milliseconds;
    unsigned ending_probe,cutscene_fixture,credits_position,credits_frames,credits_limit,board_probe_mode;
    unsigned adaptive_board_column,adaptive_board_row;
    uint16_t legacy_mode,legacy_columns,legacy_page,legacy_cursor_shape,legacy_cursor_position;
    unsigned exit_status;
    jmp_buf stop;
} Host;
static int host_save_browser(void *opaque,int mode) {
    Host *h=opaque;
    if(!h->interactive_entry || !h->window.handle || h->story_probe)return -2;
    return jx_native_game_save_browser(&h->window,&h->c,mode);
}
static Host host;
static int input_poll(void *,JxInputEvent *);
static void window_shutdown(Host *);
static void dos_allocate(void *,uint16_t,JxDosAllocateResult *);
static void dos_paragraph_allocate(void *,uint16_t,JxDosParagraphResult *);
static uint16_t bios_ticks(void *p) {Host *h=p;return h->boot_probe?(uint16_t)h->boot_seed:(uint16_t)((uint64_t)time(NULL)*182/10);}
static uint64_t fixed_cd_clock(void *p) {(void)p;return 0;}
static void key_scan(void *,uint8_t);
static void cleanup_bios(void *,uint16_t);
static void export_state(Host *,const char *);
static void sound_submit(Host *,uint16_t,JxFarPointer);
static void sound_gate(void *,uint8_t);
static void cd_query(void *,JxCdInstallation *);
static uint8_t cd_list(void *,JxFarPointer);
static void palette_capture(void *p,uint16_t first,uint16_t count,uint8_t *rgb) {
    Host *h=p;
    if(first>256 || count>256-first) {fprintf(stderr,"invalid palette capture\n");longjmp(h->stop,1);}
    memcpy(rgb,h->palette+first*3,count*3);
}
static void fail(Host *h,const char *message) {
    fprintf(stderr,"native host: %s\n",message);longjmp(h->stop,1);
}
static void palette_bios_write(void *p,JxFarPointer source,uint16_t first,uint16_t count) {
    Host *h=p;jx_palette_bios_bus_write(&h->c,&h->raw,source,first,count);
}
/* Explicit test-only inventory edits. Normal interactive games never enable
 * this flag. Every edit retains a full before/after snapshot for byte audit. */
static void test_max_supplies(Host *h,unsigned battle) {
    char before[160],after[160];unsigned edit=++h->test_supply_edits;
    if(jx_word(&h->c,0xacea))fail(h,"maximum supply preparation inside active battle");
    snprintf(before,sizeof before,"main_decompiled/build/native_r%u_supply_%03u_before.bin",h->test_supply_stage,edit);
    snprintf(after,sizeof after,"main_decompiled/build/native_r%u_supply_%03u_after.bin",h->test_supply_stage,edit);
    FILE *existing=fopen(before,"rb");if(existing){fclose(existing);fail(h,"supply evidence already exists; use a new test stage");}
    existing=fopen(after,"rb");if(existing){fclose(existing);fail(h,"supply evidence already exists; use a new test stage");}
    export_state(h,before);
    for(unsigned item=42;item<=58;item++) {
        unsigned slot=64;
        for(unsigned i=0;i<64;i++)if(jx_word(&h->c,(uint16_t)(0x878a+4*i))==item && jx_word(&h->c,(uint16_t)(0x878c+4*i))){slot=i;break;}
        if(slot==64)for(unsigned i=0;i<64;i++)if(!jx_word(&h->c,(uint16_t)(0x878c+4*i))){slot=i;break;}
        if(slot==64)fail(h,"no free inventory slot for test supplies");
        jx_set_word(&h->c,(uint16_t)(0x878a+4*slot),(uint16_t)item);
        jx_set_word(&h->c,(uint16_t)(0x878c+4*slot),32767);
    }
    export_state(h,after);
    printf("native audited maximum supplies: stage=%u edit=%u battle=%u phase=%u types=17 count=32767\n",h->test_supply_stage,edit,battle,h->walkthrough_phase);
}
/* Pausing here leaves the guest CPU and IRQ clock stopped. Input is a safe
 * basename of a host route file; authorized_supplies is an audited exception. */
static void adaptive_load_route(Host *h) {
    char line[128],path[256];
retry_route:;
    printf("native adaptive awaiting command: phase=%u\n",h->walkthrough_phase);fflush(stdout);
    if(!fgets(line,sizeof line,stdin))fail(h,"adaptive route input closed");
    line[strcspn(line,"\r\n")]=0;
    if(strcmp(line,"stop")==0)longjmp(h->stop,3);
    if(strncmp(line,"host_phase_",11)==0){
        unsigned phase;char extra;
        if(sscanf(line+11,"%u%c",&phase,&extra)!=1 || phase<92 || phase>10000)fail(h,"invalid host-only phase label");
        export_state(h,"main_decompiled/build/native_host_phase_before.bin");
        unsigned old=h->walkthrough_phase;h->walkthrough_phase=phase;
        export_state(h,"main_decompiled/build/native_host_phase_after.bin");
        printf("native host-only phase label changed: old=%u new=%u\n",old,phase);fflush(stdout);
        goto retry_route;
    }
    if(strcmp(line,"resume_prefix_39")==0){
        if(h->walkthrough_phase!=902 || jx_word(&h->c,0x9334)!=13 ||
           jx_word(&h->c,0x8760)!=56 || jx_word(&h->c,0x8762)!=31 ||
           jx_word(&h->c,0x8752)!=30 || !jx_inventory_has(&h->c,21))fail(h,"red-jade prefix resume preconditions failed");
        export_state(h,"main_decompiled/build/native_early_prefix_resume_before.bin");
        h->walkthrough_phase=39;h->walkthrough_waypoint=0;h->adaptive_ready=0;
        export_state(h,"main_decompiled/build/native_early_prefix_resume_after.bin");
        puts("native normal red-jade branch resumes compiled prefix39");fflush(stdout);return;
    }

    if(strcmp(line,"authorized_supplies")==0){
        int mainline_duel=getenv("JX_MAINLINE_BRANCH") && h->walkthrough_phase==403 && jx_word(&h->c,0x9334)==175;
        if((!mainline_duel && (h->walkthrough_phase!=436 || jx_word(&h->c,0x9334)!=192)) || jx_word(&h->c,0xacea))fail(h,"supply modification requires authorized paused boundary");
        export_state(h,mainline_duel?"main_decompiled/build/native_mainline_supplies_before.bin":"main_decompiled/build/native_authorized_supplies_before.bin");
        static const unsigned items[]={43,45,47,57};
        for(unsigned k=0;k<sizeof items/sizeof items[0];k++){
            unsigned slot=64;
            for(unsigned i=0;i<64;i++)if(jx_word(&h->c,(uint16_t)(0x878a+4*i))==items[k]){slot=i;break;}
            if(slot==64)for(unsigned i=0;i<64;i++)if(!jx_word(&h->c,(uint16_t)(0x878c+4*i))){slot=i;break;}
            if(slot==64)fail(h,"no empty inventory slot for authorized supplies");
            jx_set_word(&h->c,(uint16_t)(0x878a+4*slot),(uint16_t)items[k]);
            jx_set_word(&h->c,(uint16_t)(0x878c+4*slot),99);
            printf("native authorized supply modification: item=%u count=99 slot=%u\n",items[k],slot);
        }
        export_state(h,mainline_duel?"main_decompiled/build/native_mainline_supplies_after.bin":"main_decompiled/build/native_authorized_supplies_after.bin");
        puts("native authorized supplies complete");fflush(stdout);
        adaptive_load_route(h);return;
    }

    size_t length=strlen(line);
    if(!length || length>=sizeof h->adaptive_name)fail(h,"invalid adaptive route name");
    for(size_t i=0;i<length;i++)if(!((line[i]>='a' && line[i]<='z') || (line[i]>='0' && line[i]<='9') || line[i]=='_'))fail(h,"invalid adaptive route name");
    strcpy(h->adaptive_name,line);
    snprintf(path,sizeof path,"main_decompiled/analysis/native_adaptive_%s.command",line);
    FILE *file=fopen(path,"r");if(!file){fprintf(stderr,"native adaptive rejected unprepared route: %s\n",h->adaptive_name);export_state(h,"main_decompiled/build/native_adaptive_route_rejected.bin");goto retry_route;}
    int read=fscanf(file,"%u %u %u %u %u %u %u %u %u %u %u",&h->adaptive_expected_map,&h->adaptive_goal_map,&h->adaptive_watch_offset,&h->adaptive_watch_value,&h->adaptive_goal_x,&h->adaptive_goal_y,&h->adaptive_end_facing,&h->adaptive_end_action,&h->adaptive_skill,&h->adaptive_choice,&h->adaptive_count);
    if(read!=11 || h->adaptive_expected_map>=220 || h->adaptive_goal_map>=220 || h->adaptive_watch_offset>65535 || h->adaptive_watch_value>255 || h->adaptive_end_facing>3 || h->adaptive_end_action>258 || h->adaptive_skill>1 || h->adaptive_choice>1 || !h->adaptive_count || h->adaptive_count>4096)fail(h,"invalid adaptive route specification");
    if(jx_word(&h->c,0x9334)!=h->adaptive_expected_map)fail(h,"adaptive route starts in wrong map");
    for(unsigned i=0;i<h->adaptive_count;i++) {
        unsigned x,y;if(fscanf(file,"%u %u",&x,&y)!=2 || x>=jx_word(&h->c,0x663a) || y>=jx_word(&h->c,0x663c))fail(h,"invalid adaptive waypoint");
        h->adaptive_route[i][0]=(uint16_t)x;h->adaptive_route[i][1]=(uint16_t)y;
    }
    if(fscanf(file,"%127s",line)==1)fail(h,"unexpected adaptive route data");
    fclose(file);h->adaptive_ready=1;h->walkthrough_waypoint=0;
    printf("native adaptive route loaded: name=%s phase=%u count=%u\n",h->adaptive_name,h->walkthrough_phase,h->adaptive_count);fflush(stdout);
}

static uint32_t address(JxFarPointer p) {return ((uint32_t)p.segment*16+p.offset)&0xfffff;}
/* Explicit virtual VGA BIOS boundary for the recovered DOS CRT constructor. */
static void legacy_bios(void *opaque,JxLegacyBiosRegisters *r){
    Host *h=opaque;
    if(r->ax==0x4f00){r->ax=0x014f;return;} /* VGA, no VESA extension. */
    switch(r->ax>>8){
    case 0x00:
        if((r->ax&255)!=18)fail(h,"unsupported legacy BIOS mode");
        h->legacy_mode=18;h->legacy_columns=80;h->legacy_page=0;
        memset(h->video->planes,0,sizeof h->video->planes);
        jx_legacy_vga_model_init(&h->legacy_vga,h->video);
        h->device=jx_legacy_vga_model_bus(&h->legacy_vga);return;
    case 0x10:
        if(r->ax==0x1002){
            JxFarPointer table={r->dx,r->es};
            for(unsigned i=0;i<17;++i)if(jx_far_byte(&h->c,table,(uint16_t)i)!=i)fail(h,"unsupported legacy attribute palette mapping");
            return;
        }
        if(r->ax==0x1010){
            if(r->bx>=256)fail(h,"legacy BIOS DAC index");
            h->palette[r->bx*3]=(uint8_t)((r->dx>>8)&63);
            h->palette[r->bx*3+1]=(uint8_t)((r->cx>>8)&63);
            h->palette[r->bx*3+2]=(uint8_t)(r->cx&63);return;
        }
        if(r->ax==0x1012 || r->ax==0x1017){
            if(r->ax==0x1012){palette_bios_write(h,(JxFarPointer){r->dx,r->es},r->bx,r->cx);return;}
            if(r->bx>256 || r->cx>256-r->bx)fail(h,"legacy BIOS DAC range");
            JxFarPointer colors={r->dx,r->es};
            for(unsigned i=0;i<3u*r->cx;++i){
                if(r->ax==0x1012)h->palette[r->bx*3+i]=(uint8_t)(jx_far_byte(&h->c,colors,(uint16_t)i)&63);
                else jx_far_set_byte(&h->c,colors,(uint16_t)i,h->palette[r->bx*3+i]);
            }
            return;
        }
        fail(h,"unsupported legacy BIOS palette service");return;
    case 0x0f:r->ax=(uint16_t)((h->legacy_columns<<8)|h->legacy_mode);r->bx=(uint16_t)((r->bx&255)|(h->legacy_page<<8));return;
    case 0x1b:{
        JxFarPointer info={r->di,r->es};
        for(unsigned i=0;i<64;++i)jx_far_set_byte(&h->c,info,(uint16_t)i,0);
        jx_far_set_byte(&h->c,info,1,1);jx_far_set_byte(&h->c,info,3,0xf0);
        jx_far_set_byte(&h->c,info,0x2d,4);jx_far_set_byte(&h->c,info,0x31,3);
        r->ax=(uint16_t)((r->ax&0xff00)|0x1b);return;
    }
    case 0x0b: {
        unsigned selector=r->bx>>8;uint8_t old=h->memory[0x466];
        if(selector==0)h->memory[0x466]=(uint8_t)((old&0xe0)|(r->bx&31));
        else if(selector==1)h->memory[0x466]=(uint8_t)((old&0xdf)|((r->bx&1)<<5));
        else fail(h,"unsupported legacy CGA BIOS selector");
        return;
    }
    case 0x11:
        if((r->ax&255)>=0x22 && (r->ax&255)<=0x24)return;
        if((r->ax&255)!=0x30)fail(h,"unsupported legacy BIOS font service");
        r->cx=8;r->dx=24;r->es=0xf000;return;
    case 0x03:r->cx=h->legacy_cursor_shape;r->dx=h->legacy_cursor_position;return;
    case 0x05:
        if((r->ax&255)>=8)fail(h,"invalid legacy BIOS text page");
        h->legacy_page=r->ax&255;return;
    case 0xef:r->dx=0xffff;return;
    default:fail(h,"unsupported legacy BIOS video service");
    }
}
static void legacy_video_initialize(Host *h){
    h->legacy_mode=3;h->legacy_columns=80;h->legacy_page=0;h->legacy_cursor_shape=0x0607;h->legacy_cursor_position=0;
    h->memory[0x410]=0x20;h->memory[0x487]=0;h->memory[0x488]=9;h->memory[0x489]=0;
    h->memory[0x463]=0xd4;h->memory[0x464]=3;h->memory[0x44c]=0;h->memory[0x44d]=0x10;
    h->memory[0xf0100]=255;h->memory[0xf0101]=255;h->memory[0xf0102]=4;
    JxLegacyVideoDevice device={h,legacy_bios};
    if(!jx_legacy_video_initialize_text(&h->c,&device,&h->raw,(JxFarPointer){0x900,0x9f00}))fail(h,"DOS CRT graphics constructor failed");
}
static void name_at(Host *h,JxFarPointer p,char *out) {
    for(unsigned i=0;i<255;i++) {
        uint8_t ch=jx_far_byte(&h->c,p,(uint16_t)i);
        out[i]=ch=='\\'?'/':(char)ch;
        if(!ch)return;
    }
    fail(h,"unterminated resource filename");
}
static unsigned open_file(Host *h,JxFarPointer p) {
    char name[256];name_at(h,p,name);
    unsigned slot=0;
    for(unsigned i=1;i<128;i++)if(!h->files[i]){slot=i;break;}
    if(!slot)fail(h,"file handle table exhausted");
    h->files[slot]=fopen(name,"rb");
    if(!h->files[slot]){fprintf(stderr,"native resource open failed: %s\n",name);return 0;}
    h->next_file++;return slot;
}
static FILE *file_at(Host *h,unsigned slot) {
    if(!slot || slot>=128 || !h->files[slot])fail(h,"invalid file handle");
    return h->files[slot];
}
static unsigned open_stream(Host *h,JxFarPointer filename,JxFarPointer mode_address) {
    char mode[256],name[256];name_at(h,mode_address,mode);name_at(h,filename,name);
    int writing=strchr(mode,'w')!=NULL || strchr(mode,'a')!=NULL || strchr(mode,'+')!=NULL;
    unsigned save_index=99;char tail=0;
    int save=_strnicmp(name,"SAVE",4)==0 && name[4]>='0' && name[4]<='9' &&
        sscanf(name+4,"%u.DAT%c",&save_index,&tail)==1 && save_index<99;
    char canonical[20];snprintf(canonical,sizeof canonical,"SAVE%u.DAT",save_index);
    save=save && _stricmp(name,canonical)==0;
    if(writing && !save)fail(h,"native writes require an isolated save filename");
    if(!save)return open_file(h,filename);
    char path[128];snprintf(path,sizeof path,"main_decompiled/userdata/SAVE%u.DAT",save_index);
    unsigned slot=0;for(unsigned i=1;i<128;i++)if(!h->files[i]){slot=i;break;}
    if(!slot)fail(h,"file table exhausted");
    if(writing)(void)_mkdir("main_decompiled/userdata");
    h->files[slot]=fopen(path,mode);
    if(!h->files[slot] && !writing)return open_file(h,filename);
    if(!h->files[slot])return 0;
    h->writable[slot]=(uint8_t)writing;h->next_file++;return slot;
}
static unsigned stream_slot(Host *h,const uint16_t *a) {
    if(a[1]!=h->c.data_segment || a[0]<0x137e || (a[0]-0x1372)%12)fail(h,"invalid stream pointer");
    return (a[0]-0x1372)/12;
}
static void put_far_word(Host *h,uint16_t off,uint16_t seg,uint16_t v) {
    h->memory[((uint32_t)seg*16+off)&0xfffff]=(uint8_t)v;
    h->memory[((uint32_t)seg*16+(uint16_t)(off+1))&0xfffff]=(uint8_t)(v>>8);
}
static size_t read_file(Host *h,unsigned slot,JxFarPointer p,size_t n) {
    uint32_t at=address(p);
    if(at+n>sizeof h->memory)fail(h,"resource read exceeds guest memory");
    return fread(h->memory+at,1,n,file_at(h,slot));
}
static void map_page(Host *h,unsigned slot,unsigned page) {
    if(slot>=4 || page>=256)fail(h,"invalid EMS mapping");
    if(h->mapping[slot]>=0)memcpy(h->banks[h->mapping[slot]],h->memory+(uint32_t)h->ems_frame*16+slot*16384,16384);
    memcpy(h->memory+(uint32_t)h->ems_frame*16+slot*16384,h->banks[page],16384);
    h->mapping[slot]=(int)page;h->maps++;
}
static uint32_t dispatch(void *opaque,uint16_t s,uint16_t o,const uint16_t *a,size_t n) {
    Host *h=opaque;h->calls++;
    if(s==0x17d2) {
        if(o==0x01bc && n==1)return (uint32_t)jx_cinematic_event_180(&h->c,(int16_t)a[0],input_poll,h);
        if(o==0x01e0 && n==6){jx_transition_wave(&h->c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},a[4],a[5],input_poll,h);return 0;}
        if(o==0x01e6 && n==2){jx_transition_cascade(&h->c,a[0],a[1]);return 0;}
        if(o==0x01ec && n==2){jx_transition_outward(&h->c,(JxFarPointer){a[0],a[1]});return 0;}
        if(o==0x01f2 && n==2){jx_transition_inward(&h->c,(JxFarPointer){a[0],a[1]});return 0;}
        if(o==0x01fe && n==2){jx_transition_interlaced(&h->c,a[0],a[1]);return 0;}
        if(o==0x01f8 && n==3){jx_transition_rows(&h->c,(JxFarPointer){a[0],a[1]},a[2]);return 0;}
        if(o==0x0204 && n==3){jx_transition_columns(&h->c,(JxFarPointer){a[0],a[1]},a[2]);return 0;}
        /* Original005 passes only the image. Its callee reads the saved DI
         * of the opening prologue as mode. Original6700 sets that DI to a
         * stack address (audit: FEB0), so both reveals alternate. Restrict
         * this ABI adaptation to the actual opening, retaining strict arity
         * for any other caller. See opening_saved_mode_verification.json. */
        if(h->opening_depth && n==2 && (o==0x01f8 || o==0x0204)){
            if(o==0x01f8)jx_transition_rows(&h->c,(JxFarPointer){a[0],a[1]},0xfeb0);
            else jx_transition_columns(&h->c,(JxFarPointer){a[0],a[1]},0xfeb0);
            return 0;
        }
    }
    if(s==0x2f28 && o==0xa58a && n==4) {
        if(a[2]>256 || a[3]>256-a[2])fail(h,"invalid BIOS palette read range");
        for(unsigned i=0;i<(unsigned)a[3]*3;i++)jx_far_set_byte(&h->c,(JxFarPointer){a[0],a[1]},(uint16_t)i,h->palette[a[2]*3+i]);
        return 0;
    }
    if(s==0x2f28 && o==0x7c2e && n==0) {
        /* Original entry is BIOS AX101B BX0 CX256 only. Device grayscale
         * rounding/clamping follows VGA BIOS/DOSBox-X int10_pal.cpp:
         * https://dosbox-x.com/doxygen/html/int10__pal_8cpp_source.html */
        jx_palette_bios_grayscale(h->palette);
        return 0;
    }
    if(s==0x2f28 && o==0x7062 && n==4) {
        if(jx_story_animation_play(&h->c,(JxFarPointer){a[0],a[1]},a[2],a[3]))fail(h,"story animation reported divide fault");
        return 0;
    }
    if(s==0x1503) {
        if(o==6 && n==0)return 0;
        if(o==0x1d && n==2){put_far_word(h,a[0],a[1],h->ems_frame);return 0;}
        if(o==0x3a && n==4){put_far_word(h,a[0],a[1],256);put_far_word(h,a[2],a[3],256);return 0;}
        if(o==0x5d && n==3){put_far_word(h,a[1],a[2],7);return 0;}
        if(o==0x7d && n==3){map_page(h,a[0],a[1]);return 0;}
        if(o==0x9f && n==1) {
            if(a[0]!=7)return 0x83;
            for(unsigned i=0;i<4;i++)if(h->mapping[i]>=0) {
                memcpy(h->banks[h->mapping[i]],h->memory+(uint32_t)h->ems_frame*16+i*16384,16384);h->mapping[i]=-1;
            }
            return 0;
        }
    }
    if(s==0x1000) {
        if(o==0x2b6c && n==2) {
            /* ANM playback waits on the clock without polling input. Keep
             * painting, timer/audio IRQs and close handling alive there. */
            if(!h->story_probe && h->window.handle && !jx_native_window_pump(&h->window))window_shutdown(h);
            uint32_t seconds;uint16_t milliseconds;
            if(h->story_probe) {
                uint32_t now=h->probe_clock_milliseconds++;
                seconds=now/1000;milliseconds=(uint16_t)(now%1000);
            } else {
                struct timespec now;if(timespec_get(&now,TIME_UTC)!=TIME_UTC)fail(h,"native clock unavailable");
                seconds=(uint32_t)now.tv_sec;milliseconds=(uint16_t)(now.tv_nsec/1000000);
            }
            put_far_word(h,a[0],a[1],(uint16_t)seconds);put_far_word(h,(uint16_t)(a[0]+2),a[1],(uint16_t)(seconds>>16));
            put_far_word(h,(uint16_t)(a[0]+4),a[1],milliseconds);put_far_word(h,(uint16_t)(a[0]+6),a[1],0);put_far_word(h,(uint16_t)(a[0]+8),a[1],0);
            return 0;
        }
        if(o==0x2fa0 && n==0)return jx_clock_milliseconds(&h->c,(JxFarPointer){0,0x9e00});
        if(o==0x32da && n==3) {
            return jx_dos_allocate_paragraphs(&h->c,a[0],(JxFarPointer){a[1],a[2]},dos_paragraph_allocate,h);
        }
        if(o==0x01eb && n==1){h->exit_status=a[0];longjmp(h->stop,2);}
        if(o==0x2a8c && n>=4) {
            for(size_t i=4;i<n;i++)put_far_word(h,(uint16_t)(2*(i-4)),0x9b00,a[i]);
            return jx_sprintf_format(&h->c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},(JxFarPointer){0,0x9b00},(JxFarPointer){0,0x9c00});
        }
        if(o==0xa80 && (n==2 || n==3)) {
            /* Startup console diagnostics are explicit host presentation. */
            char text[256];name_at(h,(JxFarPointer){a[0],a[1]},text);
            fprintf(stdout,"startup[%04x]%s\n",a[0],n==3?" (numeric argument)":"");return 0;
        }
        if(o==0x738 && n==4){
            unsigned slot=open_stream(h,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]});
            if(slot && h->sound_enabled)jx_ds(&h->c)[0x1372+12*slot+10]&=(uint8_t)~0x10;
            return slot?((uint32_t)h->c.data_segment<<16 | (0x1372+12*slot)):0;
        }
        if(o==0x1ef6 && n==2){
            unsigned slot=stream_slot(h,a);int value=fgetc(file_at(h,slot));
            if(value<0)jx_ds(&h->c)[a[0]+10]|=0x10;
            return value<0?0xffff:(unsigned)value;
        }
        if(o==0x8e8 && n==6) {
            unsigned slot=stream_slot(h,a+4);if(!h->writable[slot])fail(h,"write on read-only original resource");
            uint32_t at=address((JxFarPointer){a[0],a[1]});size_t count=(size_t)a[2]*a[3];
            if(at+count>sizeof h->memory)fail(h,"save write exceeds guest memory");
            size_t wrote=fwrite(h->memory+at,1,count,file_at(h,slot));return a[2]?(uint32_t)(wrote/a[2]):0;
        }
        if(o==0x754 && n==6){size_t got=read_file(h,stream_slot(h,a+4),(JxFarPointer){a[0],a[1]},(size_t)a[2]*a[3]);return a[2]?(uint32_t)(got/a[2]):0;}
        if(o==0x27fa && n==5){int32_t offset=(int32_t)((uint32_t)a[2]|(uint32_t)a[3]<<16);return fseek(file_at(h,stream_slot(h,a)),offset,a[4])?65535:0;}
        if(o==0x636 && n==2){unsigned slot=stream_slot(h,a);int result=fclose(file_at(h,slot));h->files[slot]=NULL;h->writable[slot]=0;return result?65535:0;}
        if(o==0x2a1a && n==2){FILE *f=file_at(h,stream_slot(h,a));int lo=fgetc(f),hi=fgetc(f);return lo<0 || hi<0?65535:(unsigned)lo|((unsigned)hi<<8);}
    }
    if(s==0x150f && o==6 && n==4){jx_palette_write(&h->c,&h->raw,(JxFarPointer){a[0],a[1]},a[2],a[3]);return 0;}
    if(s==0x2f28 && o==0xa572 && n==4){jx_palette_bios_write((JxFarPointer){a[0],a[1]},a[2],a[3],palette_bios_write,h);return 0;}
    if(s==0x2f28 && (o==0x3f66 || o==0x415e) && n==3) {
        JxPaletteBusAdapter adapter={&h->raw,palette_capture,h};
        JxPaletteIO io=jx_palette_bus_io(&adapter);
        JxPaletteFlow flow=o==0x3f66?jx_palette_fade_black(&h->c,(JxFarPointer){a[0],a[1]},(int16_t)a[2],&io):jx_palette_fade_to(&h->c,(JxFarPointer){a[0],a[1]},(int16_t)a[2],&io);
        if(flow)fail(h,"palette fade reported divide fault");return 0;
    }
    if(s==0x1f2a && o==0x06f2 && n==3){jx_font_set_colors(&h->c,a[0],a[1],a[2]);return 0;}
    if(s==0x1f2a && o==0x0718 && n==1){jx_font_set_mode(&h->c,a[0]);return 0;}
    if(s==0x2f28 && o==0x6572 && n==6){if(jx_text_pages(&h->c,a[0],a[1],a[2],(JxFarPointer){a[3],a[4]},a[5],(JxFarPointer){0x700,0x9f00}))fail(h,"original text page divide fault");return 0;}
    if(s==0x1f2a && o==0x0e04 && n==5){JxTextResult result=jx_font_draw_text_timed(&h->c,a[0],a[1],(JxFarPointer){a[2],a[3]},a[4]);if(result.flow==JX_GLYPH_STALLED)fail(h,"timed glyph raster stalled");return (uint16_t)result.result;}
    if(s==0x1f2a && o==0x0728 && n==4) {
        JxTextResult result=jx_font_draw_text(&h->c,a[0],a[1],(JxFarPointer){a[2],a[3]});
        if(result.flow)fail(h,"font drawing stalled");return (uint16_t)result.result;
    }
    if(s==0x1f2a && n==0) {
        if(o==0x5528){jx_configuration_load(&h->c);return 0;}
        if(o==0x5436){if(jx_boot_memory_initialize(&h->c,bios_ticks,h))fail(h,"boot memory initialization terminated");return 0;}
        if(o==0xae40){jx_game_main_loop(&h->c,(JxFarPointer){0,0x9600});return 0;}
        if(o==0x5b56){if(jx_resources_initialize(&h->c,(JxFarPointer){0x400,0x9000}))fail(h,"resource initialization terminated");return 0;}
        if(o==0x60ac){jx_game_database_load(&h->c);return 0;}
        if(o==0x629e){jx_new_game_initialize(&h->c);return 0;}
        if(o==0xca04){jx_scene_presence_load(&h->c);return 0;}
    }
    if(s==0x1832 && o==0x2098 && n==2){
        JxCdDiscovery discovery={h,cd_query,cd_list};
        return jx_cd_initialize_current(&h->c,&discovery,(JxFarPointer){a[0],a[1]});
    }
    if(s==0x1f2a && o==0xc24e && n==0)return jx_event_clear_state(&h->c);
    if(s==0x2f28 && o==0x856c && n==0){jx_music_poll(&h->c);return 0;}
    if(s==0x2f28 && o==0x753a && n==2){jx_transition_border_passes(&h->c,(JxFarPointer){a[0],a[1]});return 0;}
    if(s==0x1f2a && o==0x720e && n==1){jx_wait_milliseconds(&h->c,a[0]);return 0;}
    if(s==0x1f2a && o==0x9d34 && n==0){jx_music_toggle(&h->c);return 0;}
    if(s==0x1f2a && o==0x9d6c && n==0){jx_sound_toggle(&h->c);return 0;}
    if(s==0x1f2a && o==0x9d80 && n==0){jx_music_restart(&h->c);return 0;}
    if(s==0x1f2a && o==0x7606 && n==0){jx_party_interact(&h->c);return 0;}
    if(s==0x1f2a && o==0xae12 && n==0){jx_party_actor_menus(&h->c);return 0;}
    if(s==0x1f2a && o==0xa892 && n==0){jx_party_skill_menus(&h->c);return 0;}
    if(s==0x1f2a && o==0xa8c0 && n==0){jx_party_use_item(&h->c);return 0;}
    if(s==0x1f2a && o==0x799a && n==1){jx_party_present_item(&h->c,a[0]);return 0;}
    if(s==0x1f2a && o==0x14ec && n==0){jx_sound_initialize(&h->c);return 0;}
    if(s==0x1f2a && o==0x2740 && n==2)return jx_sound_detect_version(&h->c,(JxFarPointer){a[0],a[1]});
    if(s==0x1f2a && o==0x27aa && n==1)return jx_sound_dsp_read(&h->c,a[0]);
    if(s==0x1f2a && o==0x266c && n==2){jx_sound_install_irq(&h->c,(JxFarPointer){a[0],a[1]},sound_gate,h);return 0;}
    if(s==0x1f2a && o==0x2602 && n==2){JxCleanupDevice device={h,sound_gate,cleanup_bios};jx_sound_restore_irq(&h->c,(JxFarPointer){a[0],a[1]},&device);return 0;}
    if(s==0x1f2a && o==0x8cea && n==2){
        if(jx_inventory_grant_formatted(&h->c,(int16_t)jx_signed_word(a[0]),(int16_t)jx_signed_word(a[1]),
            (JxFarPointer){0,0xb100},(JxFarPointer){0,0xb200},input_poll,h))fail(h,"inventory notification font stalled");
        return 0;
    }
    if(s==0x1f2a && o==0xb196 && n==0){jx_game_shutdown(&h->c);fail(h,"shutdown termination unexpectedly returned");}
    if(s==0x2f28 && o==0xa562 && n==1){JxCleanupDevice device={h,NULL,cleanup_bios};jx_bios_video_call(&device,a[0]);return 0;}
    if(s==0x1f2a && o==0xca66 && n==1) {
        JxWorldUpdateFlow flow=jx_npc_redraw_background_world(&h->c,(int16_t)jx_signed_word(a[0]),(JxFarPointer){0,0x9100});
        if(flow)fail(h,"NPC redraw reported a guest divide fault");return 0;
    }
    if(s==0x1f2a && o==0xcc24 && n==2) {
        jx_party_walk_to(&h->c,a[0],a[1],(JxFarPointer){0,0x9200});return 0;
    }
    if(s==0x2f28 && o==0x95aa && n==0){jx_scene_run_trajectories(&h->c);return 0;}
    if(s==0x2f28 && o==0x8c6e && n==0){jx_party_advance_frames(&h->c);return 0;}
    if(s==0x2f28 && o==0x8d50 && n==0){jx_party_shift_trail(&h->c);return 0;}
    if(s==0x2f28 && o==0x5166 && n==0){if(jx_party_redraw_trail_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"party trail redraw divide fault");return 0;}
    if(s==0x2f28 && o==0x5346 && n==0){if(jx_party_redraw_page_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"party page redraw divide fault");return 0;}
    if(s==0x2f28 && o==0x944c && n==3){jx_party_run_moves(&h->c,(JxFarPointer){a[0],a[1]},a[2]);return 0;}
    if(s==0x1f2a && o==0xcd1c && n==4){jx_npc_run_moves(&h->c,a[0],(JxFarPointer){a[1],a[2]},a[3]);return 0;}
    if(s==0x1f2a && o==0xcb08 && n==4)return jx_choose_step(&h->c,a[0],a[1],a[2],a[3]);
    if(s==0x1f2a && o==0xcf6a && n==3){jx_npc_walk_to(&h->c,a[0],a[1],a[2],(JxFarPointer){0,0x9f00});return 0;}
    if(s==0x2f28 && o==0x492a && n==2) {
        JxWorldUpdateFlow flow=jx_scene_cell_layers_world(&h->c,a[0],a[1],(JxFarPointer){0,0x9100});
        if(flow)fail(h,"scene collision reported a guest divide fault");return 0;
    }
    if(s==0x2f28 && o==0x86ce && n==2) {
        uint16_t result=0;
        JxWorldUpdateFlow flow=jx_movement_classify_cells(&h->c,a[0],a[1],(JxFarPointer){0,0x9100},&result);
        if(flow)fail(h,"movement classification reported a guest divide fault");return result;
    }
    if(s==0x2f28 && o==0x8dd6 && n==2) {
        JxWorldUpdateFlow flow=jx_party_move_cycle_world(&h->c,a[0],a[1],(JxFarPointer){0,0x9100});
        if(flow)fail(h,"party movement reported a guest divide fault");return 0;
    }
    if(s==0x2f28 && o==0x88de && n==0) {
        JxWorldUpdateFlow flow=jx_party_check_triggers_world(&h->c,(JxFarPointer){0,0x9100});
        if(flow)fail(h,"trigger reported a guest divide fault");return 0;
    }
    if(s==0x1f2a && o==0x6700 && n==2) {
        if(jx_map_load(&h->c,a[0],a[1],(JxFarPointer){0x300,0x9000}))fail(h,"nested map load terminated");return 0;
    }
    if(s==0x2f28 && o==0x648e && n==4) {
        JxGlyphFlow flow=jx_dialogue_show_font(&h->c,(int16_t)jx_signed_word(a[0]),
            (JxFarPointer){a[1],a[2]},a[3],(JxFarPointer){0,0x9400},input_poll,h,(JxFarPointer){0,0x9500});
        if(flow)fail(h,"dialogue glyph loop stalled");return 0;
    }
    if(s==0x2f28 && o==0x448a && n==7)return jx_map_cell_info(&h->c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},a[4],a[5],a[6]);
    if(s==0x2f28 && n==0 && (o==0x9c3a || o==0x9d42 || o==0x9efe)){
        JxFarPointer work={0x100,0x9f00};
        uint16_t fault=o==0x9c3a?jx_world_direction_tiles(&h->c,work):o==0x9d42?jx_world_sinking_tiles(&h->c,work):jx_world_lethal_tiles(&h->c,work);
        if(fault)fail(h,"world hazard DIV zero fault");return 0;
    }
    if(s==0x2f28 && o==0x7886 && n==2){jx_world_shift_page(&h->c,a[0],a[1]);return 0;}
    if(s==0x2f28 && o==0x7934 && n==2){jx_world_shift_pages(&h->c,a[0],a[1]);return 0;}
    if(s==0x1f2a && o==0xdf00 && n==0){jx_world_random_encounter(&h->c);return 0;}
    if(s==0x2f28 && o==0x9ffe && n==0){jx_world_moving_hazards(&h->c);return 0;}
    if(s==0x2f28 && o==0x866a && n==0)return jx_cd_validate_track_range(&h->c,(JxFarPointer){0,0x9f00});
    fprintf(stderr,"unconnected call %04x:%04x (%zu words)\n",s,o,n);
    if(h->sound_enabled)for(unsigned i=h->trace_position>32?h->trace_position-32:0;i<h->trace_position;i++)
        fprintf(stderr,"trace %04x:%04x\n",h->trace_segment[i%32],h->trace_offset[i%32]);
    fail(h,"engine integration needs this call");return 0;
}
static void dos_open(void *p,JxFarPointer filename,uint8_t mode,JxDosWriteResult *r) {
    Host *h=p;if(mode)fail(h,"unsupported DOS resource access");unsigned slot=open_file(h,filename);r->ax=slot?(uint16_t)slot:2;r->carry=(uint8_t)(slot==0);
}
static void dos_read(void *p,uint16_t handle,JxFarPointer buffer,uint16_t count,JxDosWriteResult *r) {
    r->ax=(uint16_t)read_file(p,handle,buffer,count);r->carry=0;
}
static void dos_seek(void *p,uint16_t handle,uint8_t origin,uint32_t offset,JxDosSeekResult *r) {
    Host *h=p;FILE *f=file_at(h,handle);r->carry=(uint8_t)(fseek(f,(int32_t)offset,origin)!=0);
    long at=ftell(f);r->ax=(uint16_t)at;r->dx=(uint16_t)((uint32_t)at>>16);
}
static void dos_close(void *p,uint16_t handle,JxDosWriteResult *r) {
    Host *h=p;r->carry=(uint8_t)(fclose(file_at(h,handle))!=0);r->ax=0;h->files[handle]=NULL;
}
static void dos_allocate(void *p,uint16_t paragraphs,JxDosAllocateResult *r) {
    Host *h=p;
    if(h->ems_frame!=0xd000)fail(h,"DOS allocation requires boot heap topology");
    unsigned run=0,start=0;
    for(unsigned i=0;i<0x4000;i++) {
        if(h->heap_owner[i])run=0;else {if(!run)start=i;run++;}
        if(paragraphs && run>=paragraphs) {
            uint16_t segment=(uint16_t)(0x5000+start);
            for(unsigned j=start;j<start+paragraphs;j++)h->heap_owner[j]=segment;
            h->heap_size[start]=paragraphs;r->segment=segment;r->carry=0;return;
        }
    }
    r->segment=0;r->carry=1;
}
static void dos_paragraph_allocate(void *p,uint16_t paragraphs,JxDosParagraphResult *out) {
    Host *h=p;JxDosAllocateResult result;dos_allocate(h,paragraphs,&result);
    unsigned best=0,run=0;
    if(result.carry)for(unsigned i=0;i<0x4000;i++){run=h->heap_owner[i]?0:run+1;if(run>best)best=run;}
    *out=(JxDosParagraphResult){result.carry?8:result.segment,(uint16_t)best,result.carry};
}
static void dos_free(void *p,uint16_t segment) {
    Host *h=p;if(segment<0x5000 || segment>=0x9000)fail(h,"invalid DOS free segment");
    unsigned start=segment-0x5000,count=h->heap_size[start];
    if(!count || h->heap_owner[start]!=segment)fail(h,"DOS free is not an allocation start");
    for(unsigned i=start;i<start+count;i++)h->heap_owner[i]=0;
    h->heap_size[start]=0;
}
static void video_out(void *p,uint16_t port,uint16_t value,unsigned width) {
    Host *h=p;
    if(port==0x3c8 && width==1){h->dac=(value&255)*3;return;}
    if(port==0x3c9 && width==1){
        h->palette[h->dac++%768]=(uint8_t)value;
        if(h->dac%768==0 && h->window.handle && !h->story_probe)jx_native_window_present(&h->window);
        return;
    }
    h->device.out(h->device.opaque,port,value,width);
    if(h->video->error)fail(h,"unsupported VGA port operation");
}
static uint8_t video_in(void *p,uint16_t port) {
    Host *h=p;
    if(port==0x3da){
        if(h->window.handle && !h->story_probe && !jx_native_window_wait_retrace(&h->window))window_shutdown(h);
        return 8;
    }
    return h->device.in_byte(h->device.opaque,port);
}
static uint8_t video_read(void *p,uint32_t at){Host *h=p;at&=0xfffffu;if(at<0xa0000 || at>=0xb0000)return h->memory[at];uint8_t v=h->device.read_byte(h->device.opaque,at);if(h->video->error){fprintf(stderr,"native VGA read diagnostic: address=%08x error=%u map=%u position=(%u,%u) mode=%u read_map=%u\n",(unsigned)at,(unsigned)h->video->error,(unsigned)jx_word(&h->c,0x9334),(unsigned)jx_word(&h->c,0x8760),(unsigned)jx_word(&h->c,0x8762),(unsigned)h->video->mode,(unsigned)h->video->read_map);fail(h,"unsupported VGA read");}return v;}
static void video_write(void *p,uint32_t at,uint8_t v){Host *h=p;at&=0xfffffu;if(at<0xa0000 || at>=0xb0000){h->memory[at]=v;return;}h->device.write_byte(h->device.opaque,at,v);if(h->video->error)fail(h,"unsupported VGA write");}
static void native_video_bios(void *p,uint16_t mode){
    Host *h=p;jx_video_startup_model_bios(&h->startup,mode);
    if(h->video->error)fail(h,"unsupported native video mode");
    h->legacy_mode=mode;h->device=jx_video_startup_model_bus(&h->startup);
}
static uint16_t legacy_palette(void *,uint16_t,JxFarPointer);
static void legacy_near(void *p,uint16_t entry,uint16_t value){
    Host *h=p;
    if(entry==0x37e7)return;
    if(entry==0x3331){JxLegacyVideoDevice device={h,legacy_bios};(void)jx_legacy_video_select_cga_palette(&h->c,value,&device,legacy_palette,h);return;}
    if(entry==0x5b64){(void)jx_legacy_video_planar_color(&h->c,value);return;}
    if(entry==0x5b5b){(void)jx_legacy_video_planar_down(&h->c,0);return;}
    if(entry==0x38c5){jx_legacy_video_reset_registers(&h->c,&h->raw);return;}
    fail(h,"unsupported legacy image near callback");
}
static void legacy_point(void *p,uint16_t entry,uint16_t x,uint16_t y){
    Host *h=p;if(entry!=0x5b06)fail(h,"unsupported legacy point callback");
    jx_legacy_video_planar_address(&h->c,x,y);
}
static void legacy_bank(void *p,JxFarPointer target,uint16_t bx,uint16_t dx){
    Host *h=p;(void)bx;(void)dx;
    if(target.segment!=0x1832 || target.offset!=0x309d)fail(h,"unsupported legacy bank callback");
}
static uint16_t legacy_palette(void *p,uint16_t entry,JxFarPointer colors){
    Host *h=p;if(entry==0x37e7)return 1;
    if(entry!=0x654b)fail(h,"unsupported legacy palette callback");
    JxLegacyVideoDevice device={h,legacy_bios};
    jx_legacy_video_palette_block(&h->c,&device,colors,(JxFarPointer){0xa00,0x9f00});return 0;
}
static void cleanup_bios(void *p,uint16_t mode) {
    Host *h=p;
    if(mode==3){jx_native_window_close(&h->window);return;}
    native_video_bios(h,mode);
}
static uint8_t port_read(void *p,uint16_t port) {
    Host *h=p;if(port==0x60)return h->scan_code;if(port==0x61)return h->keyboard_control;
    if(h->sound_enabled){
        if(port==0x21 || port==0xa1)return h->pic[port==0xa1];
        uint16_t base=jx_word(&h->c,0x31b8);
        if(port==(uint16_t)(base+5))return h->audio_irq_pending;
        if(port==(uint16_t)(base+15)){h->audio_irq_pending=0;return 0;}
        if(port==(uint16_t)(base+12))return 0;
        if(port==(uint16_t)(base+14)){h->audio_irq_pending=0;return h->dsp_cursor<h->dsp_count?0x80:0;}
        if(port==(uint16_t)(base+10)){
            if(h->dsp_cursor>=h->dsp_count)fail(h,"DSP read queue exhausted");
            return h->dsp_queue[h->dsp_cursor++];
        }
    }
    fail(h,"unsupported input hardware port read");return 0;
}
static void port_write(void *p,uint16_t port,uint8_t value) {
    Host *h=p;if(port==0x61){h->keyboard_control=value;return;}
    if((port==0x20 || port==0xa0) && value==0x20)return;
    if(h->sound_enabled){
        if(port==0x21 || port==0xa1){h->pic[port==0xa1]=value;return;}
        uint16_t base=jx_word(&h->c,0x31b8);
        if(port==(uint16_t)(base+6)){
            if(value){h->audio_irq_pending=0;jx_native_audio_close(&h->audio);h->dsp_cursor=h->dsp_count=h->dsp_arguments=0;}
            else {h->dsp_queue[0]=0xaa;h->dsp_cursor=0;h->dsp_count=1;}
            return;
        }
        if(port==(uint16_t)(base+12)){
            if(h->dsp_arguments)h->dsp_arguments--;
            else if(value==0xe1){h->dsp_queue[0]=4;h->dsp_queue[1]=5;h->dsp_cursor=0;h->dsp_count=2;}
            else if(value==0x41 || value==0x14 || value==0x48)h->dsp_arguments=2;
            else if(value==0xc0 || value==0xc4 || value==0xb0 || value==0xb4)h->dsp_arguments=3;
            else if(value==0x40)h->dsp_arguments=1;
            return;
        }
        if(port==base+4 || port==base+5)return;
        if(port<256 && (port<8 || (port>=10 && port<=12) || port==0x82 || port==0x83 || port==0x87 ||
            port==0x8a || port==0x8b || port==0x89 || (port>=0xc4 && port<=0xce) || port==0xd4 || port==0xd6 || port==0xd8)){
            h->dma_ports[port]=value;return;
        }
    }
    fail(h,"unsupported input hardware port write");
}
static void keyboard_bios(void *p,uint16_t ax,uint16_t bx) {
    if(ax!=0x0305 || bx!=0)fail(p,"unsupported keyboard BIOS operation");
    /* Native key repeat comes from Windows; this DOS typematic request is
     * acknowledged by the device adapter, not dispatched as game logic. */
}
static uint32_t get_vector(void *p,uint8_t number){return ((Host *)p)->vectors[number];}
static void set_vector(void *p,uint8_t number,JxFarPointer vector) {
    ((Host *)p)->vectors[number]=(uint32_t)vector.segment<<16|vector.offset;
}
static void interrupt_disable(void *p){(void)p; /* All producers run serially on the window thread. */}
static void sound_gate(void *p,uint8_t enabled){(void)p;(void)enabled; /* Serialized native producers. */}
static void key_scan(void *p,uint8_t scan) {
    Host *h=p;h->scan_code=scan;
    jx_keyboard_irq(&h->c,&h->ports,interrupt_disable,h,(JxFarPointer){0,0x9300});
}
static void timer_tick(void *p) {
    Host *h=p;
    if(h->sound_enabled && h->window.handle && jx_native_audio_take_completion(&h->audio)){
        h->audio_irq_pending=jx_word(&h->c,0xd48e)==8?1:2;
        jx_sound_irq(&h->c);h->audio_irqs++;
    }
    jx_timer_irq(&h->c,(JxFarPointer){0,0x9300});
}
static int board_probe_poll(void *p,JxInputEvent *event){
    Host *h=p;uint16_t target_x=jx_word(&h->c,0x3462),target_y=jx_word(&h->c,0x3464);
    /* Probe cursor state is private. Input follows the answer read by the
     * actual puzzle loader; no game board/answer data is modified. */
    uint16_t x=jx_far_word(&h->c,0x9f00,0xe00),y=jx_far_word(&h->c,0x9f00,0xe02);
    if(target_x>18 || target_y>18)fail(h,"invalid board puzzle answer");
    if(h->window.handle && !h->window.planar_paints){
        for(unsigned step=0;step<100 && !h->window.planar_paints;++step)
            if(!jx_native_window_pump(&h->window))fail(h,"board window closed before planar paint");
        if(!h->window.planar_paints)fail(h,"board window did not paint640x480");
    }
    if(!h->memory[0x9fe04]){
        h->memory[0x9fe04]=1;FILE *image=fopen("main_decompiled/build/native_board.ppm","wb");
        if(!image)fail(h,"cannot export board image");
        fprintf(image,"P6\n640 480\n255\n");
        for(unsigned py=0;py<480;++py)for(unsigned px=0;px<640;++px){
            unsigned color=jx_legacy_vga_model_pixel(&h->legacy_vga,(uint16_t)px,(uint16_t)py);
            uint8_t rgb[3];for(unsigned channel=0;channel<3;++channel)rgb[channel]=(uint8_t)(h->palette[color*3+channel]*4);
            if(fwrite(rgb,1,3,image)!=3)fail(h,"cannot write board image");
        }
        if(fclose(image))fail(h,"cannot close board image");
    }
    uint16_t scan;
    if(x<target_x){scan=0x4d;++x;}else if(x>target_x){scan=0x4b;--x;}
    else if(y<target_y){scan=0x50;++y;}else if(y>target_y){scan=0x48;--y;}
    else scan=0x1c;
    h->memory[0x9fe00]=(uint8_t)x;h->memory[0x9fe01]=(uint8_t)(x>>8);
    h->memory[0x9fe02]=(uint8_t)y;h->memory[0x9fe03]=(uint8_t)(y>>8);
    *event=(JxInputEvent){0,scan,0,0};++h->story_poll_steps;return 0;
}
/* Drive the actual board cursor through IRQ1 and the recovered input queue.
 * The original loader's square is read-only; no board/state/result writes. */
static int adaptive_board_poll(void *p,JxInputEvent *event){
    Host *h=p;unsigned x=h->adaptive_board_column,y=h->adaptive_board_row;
    unsigned target_x=jx_word(&h->c,0x3462),target_y=jx_word(&h->c,0x3464);
    if(target_x>18 || target_y>18)fail(h,"invalid adaptive board answer");
    uint8_t scan;
    if(x<target_x){scan=0x4d;++x;}else if(x>target_x){scan=0x4b;--x;}
    else if(y<target_y){scan=0x50;++y;}else if(y>target_y){scan=0x48;--y;}
    else scan=0x1c;
    key_scan(h,scan);key_scan(h,(uint8_t)(scan|0x80));
    for(unsigned attempts=0;attempts<64;attempts++){
        ++h->story_poll_steps;
        if(jx_input_poll_queue(&h->c,event)==0 && event->type==0){
            if(event->value1!=scan)fail(h,"adaptive board received unexpected key");
            h->adaptive_board_column=x;h->adaptive_board_row=y;return 0;
        }
    }
    fail(h,"adaptive board key not delivered through IRQ queue");return 1;
}
static int adaptive_best_skill(Host *h,uint16_t actor) {
    if(actor>4)return -1;
    uint16_t stats=(uint16_t)(0xd04e + 68*actor),mp=jx_word(&h->c,(uint16_t)(stats+0x12));
    /* Original battle28 permits a ten-turn dialogue resolution. Use ordinary
     * attacks rather than spending the pursuer's damaging learned skills. */
    if(actor==3 && jx_word(&h->c,0x2934)==28)return -1;
    int best=-1;unsigned power=0;
    /* Boss 7 can outlast greedy strongest-first casting. Plan up to four
     * ordinary menu selections against the actual remaining MP; no actor,
     * inventory or combat state is changed by this keyboard policy. */
    if(jx_word(&h->c,0x2934)==7 && mp<=1024){
        unsigned value[5][1025]={{0}};
        for(unsigned turns=1;turns<=4;turns++)for(unsigned budget=0;budget<=mp;budget++){
            for(unsigned slot=0;slot<16;slot++){
                unsigned id=jx_ds(&h->c)[(uint16_t)(stats+0x34+slot)];
                if(id>=64)continue;
                uint16_t skill=(uint16_t)(0x5154+28*id);
                unsigned cost=jx_word(&h->c,(uint16_t)(skill+0x14));
                unsigned damage=jx_word(&h->c,(uint16_t)(skill+0x10));
                if(cost && cost<=budget && damage+value[turns-1][budget-cost]>value[turns][budget])
                    value[turns][budget]=damage+value[turns-1][budget-cost];
            }
        }
        for(unsigned slot=0;slot<16;slot++){
            unsigned id=jx_ds(&h->c)[(uint16_t)(stats+0x34+slot)];
            if(id>=64)continue;
            uint16_t skill=(uint16_t)(0x5154+28*id);
            unsigned cost=jx_word(&h->c,(uint16_t)(skill+0x14));
            unsigned damage=jx_word(&h->c,(uint16_t)(skill+0x10));
            if(cost && cost<=mp && damage+value[3][mp-cost]==value[4][mp] && damage>power){best=(int)slot;power=damage;}
        }
        return best;
    }
    for(unsigned slot=0;slot<16;slot++){
        unsigned id=jx_ds(&h->c)[(uint16_t)(stats+0x34+slot)];
        if(id>=64)continue;
        /* Original battle 15 advances its story only when Dugu uses the
           learned Tianmo skill. Select that ordinary skill-menu slot. */
        if(actor==0 && jx_word(&h->c,0x2934)==15 && id==30)return (int)slot;
        uint16_t skill=(uint16_t)(0x5154+28*id);
        unsigned damage=jx_word(&h->c,(uint16_t)(skill+0x10));
        if(jx_word(&h->c,(uint16_t)(skill+0x14))<=mp && damage>power){best=(int)slot;power=damage;}
    }
    return best;
}
/* Read-only conservative finishing estimate for the original two-character
 * battle25. At most the carried tea may fund their strongest learned skills;
 * the minimum original damage scale is90 percent. */
static int adaptive_can_finish_with_carried_tea(Host *h) {
    if(jx_word(&h->c,0x2934)!=25 || jx_word(&h->c,0x2936)!=0 || jx_word(&h->c,0x2938)!=1)return 0;
    unsigned tea=0,needed=0,power=0;
    for(unsigned i=0;i<64;i++)
        if(jx_word(&h->c,(uint16_t)(0x878a+4*i))==47)tea+=jx_word(&h->c,(uint16_t)(0x878c+4*i));
    for(unsigned actor=0;actor<2;actor++){
        /* While selecting the companion, the first actor has already paid
         * MP for the retained command. Count its selected damage directly. */
        if(actor==0 && jx_word(&h->c,0x2930)==1){
            unsigned selected=jx_word(&h->c,0x61d8);
            if(jx_word(&h->c,0x61d6)!=3 || selected>=64)return 0;
            power+=jx_word(&h->c,(uint16_t)(0x5154+28*selected+0x10));
            continue;
        }
        uint16_t stats=(uint16_t)(0xd04e + 68*actor);
        unsigned mp=jx_word(&h->c,(uint16_t)(stats+0x12)),maximum=jx_word(&h->c,(uint16_t)(stats+0x16));
        unsigned best=0,best_cost=0;
        for(unsigned slot=0;slot<16;slot++){
            unsigned id=jx_ds(&h->c)[(uint16_t)(stats+0x34+slot)];
            if(id>=64)continue;
            uint16_t skill=(uint16_t)(0x5154+28*id);
            unsigned damage=jx_word(&h->c,(uint16_t)(skill+0x10)),cost=jx_word(&h->c,(uint16_t)(skill+0x14));
            if(cost<=maximum && cost<=mp+100*tea && damage>best){best=damage;best_cost=cost;}
        }
        if(best==0)return 0;
        if(best_cost>mp)needed+=(best_cost-mp+99)/100;
        power+=best;
    }
    return needed<=tea && jx_word(&h->c,0x6614)<=power*90/100;
}
static int ui_test_input(Host *h,JxInputEvent *event) {
    if(h->ui_index>=h->ui_count)fail(h,"menu regression exhausted input");
    char path[160];snprintf(path,sizeof path,"main_decompiled/build/menu_%s_%02u.ppm",h->ui_name,h->ui_index);
    FILE *f=fopen(path,"wb");if(!f)fail(h,"menu screenshot failed");fprintf(f,"P6\n320 200\n255\n");
    unsigned start=(unsigned)h->video->start_high<<8;
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++) {
        unsigned color=h->video->planes[x&3][(start+y*80+x/4)&65535];
        for(unsigned rgb=0;rgb<3;rgb++)fputc(h->palette[color*3+rgb]*4,f);
    }
    if(fclose(f))fail(h,"menu screenshot close failed");
    *event=(JxInputEvent){h->ui_events[h->ui_index][0],h->ui_events[h->ui_index][1],0,0};h->ui_index++;return 0;
}
static uint32_t native_input_dispatch(void *p,uint16_t s,uint16_t o,const uint16_t *a,size_t n) {
    Host *h=p;
    if(h->ui_events && s==0x1f2a && o==0xc1f2 && n==2){JxInputEvent event;ui_test_input(h,&event);uint32_t at=((uint32_t)a[1]*16+a[0])&0xfffff;h->memory[at]=(uint8_t)event.type;h->memory[at+1]=0;h->memory[at+2]=(uint8_t)event.value1;h->memory[at+3]=0;memset(h->memory+at+4,0,4);return 0;}
    if(h->test_supply_stage && s==0x1f2a && o==0x4934 && n==3)test_max_supplies(h,a[0]);
    int adaptive_tactics=h->adaptive_walkthrough && h->walkthrough_phase>=92;
    if(h->adaptive_walkthrough && h->walkthrough_phase>=190 && jx_word(&h->c,0x9334)==81){
        unsigned call=++h->adaptive_map81_calls;
        if(call<=48 || call%1048576==0){
            fprintf(stderr,"adaptive map81 dispatch call=%u %04X:%04X argc=%u arg0=%u ticks=%u world=%u direction=%u story=%u\n",call,s,o,(unsigned)n,n?a[0]:0,h->entry_ticks,jx_word(&h->c,0x585e),jx_word(&h->c,0x2942),h->story_probe);
            if(call%1048576==0)export_state(h,"main_decompiled/build/native_adaptive_map81_wait_observation.bin");
        }
    }
    int adaptive_scene_trigger=0;
    int adaptive_scene_entry=0;
    int adaptive_scene_contact=0;
    if(adaptive_tactics && h->entry_world){
        unsigned map=jx_word(&h->c,0x9334);
        if(map<220){
            uint16_t entry=(uint16_t)(0x0a6c+4*map);
            adaptive_scene_entry=s==jx_word(&h->c,(uint16_t)(entry+2)) && o==jx_word(&h->c,entry);
            for(unsigned i=0;i<30;i++){
                uint16_t npc=(uint16_t)(0x889c+86*i);
                if(jx_ds(&h->c)[(uint16_t)(0x665a+30*map+i)]!=1)continue;
                if(s==jx_word(&h->c,(uint16_t)(npc+84)) && o==jx_word(&h->c,(uint16_t)(npc+82)) && !(s==0x2f28 && o==0x86c4))adaptive_scene_contact=1;
            }
        }
        for(unsigned i=0;i<10;i++){
            uint16_t trigger=(uint16_t)(0x6584+8*i);
            if(jx_word(&h->c,trigger)==999 || jx_word(&h->c,(uint16_t)(trigger+2))==999)continue;
            if(s==jx_word(&h->c,(uint16_t)(trigger+6)) && o==jx_word(&h->c,(uint16_t)(trigger+4)) && !(s==0x2f28 && o==0x86ba))adaptive_scene_trigger=1;
        }
    }
    if(adaptive_tactics && h->adaptive_end_action<3 && h->entry_world && !h->battle_input && s==0x1f2a && o==0x80fa && n==5 &&
       a[0]==248 && a[1]==0 && a[2]==3 && jx_word(&h->c,0x8722)==0xae12 &&
       jx_word(&h->c,0x8726)==0xa8c0 && jx_word(&h->c,0x872a)==0xa892){
        unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
        h->menu_probe=3;h->input_probe_step=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        unsigned polls=h->input_probe_step;h->menu_probe=prior_menu;h->input_probe_step=prior_step;
        if((uint16_t)result!=0xffff)fail(h,"stray world action menu did not cancel normally");
        fprintf(stderr,"native adaptive world action menu canceled by Escape: polls=%u map=%u\n",polls,jx_word(&h->c,0x9334));
        return result;
    }
    if(adaptive_tactics && h->adaptive_skill && h->battle_input && jx_word(&h->c,0xacea) && s==0x1f2a){
        uint16_t actor=jx_word(&h->c,jx_word(&h->c,0x2930)==0?0x2936:0x2938);
        unsigned recovery_item=0;
        /* Original 3F12 recovery items set action2 and re-open the menu;
         * they do not advance a turn. Use only actual carried supplies. */
        if(actor==1 && jx_word(&h->c,0x2934)==18){
            if(jx_word(&h->c,0xd0a4)<100 && jx_inventory_has(&h->c,57))recovery_item=57;
            if(jx_word(&h->c,0xd0a2)<300){
                if(jx_inventory_has(&h->c,51))recovery_item=51;
                else if(jx_inventory_has(&h->c,48))recovery_item=48;
            }
        }
        if(actor==1 && jx_word(&h->c,0xd0a4)<50 && jx_inventory_has(&h->c,57))recovery_item=57;
        if(actor<5 && jx_inventory_has(&h->c,56)){
            uint16_t stats=(uint16_t)(0xd04e + 68*actor);
            if(jx_word(&h->c,(uint16_t)(stats+0x12))<50)recovery_item=56;
            if(jx_word(&h->c,(uint16_t)(stats+0x10))<250 && jx_inventory_has(&h->c,53))recovery_item=53;
        }
        /* Five-color hall is an original five-fight chain with poison.
         * Spend real carried recovery supplies through the normal item menu. */
        if(actor<5 && jx_word(&h->c,0x9334)==173 &&
            (jx_word(&h->c,0x2934)==22 || jx_word(&h->c,0x2934)==21)){
            uint16_t stats=(uint16_t)(0xd04e + 68*actor);
            if(jx_word(&h->c,(uint16_t)(stats+0x12))<100){
                static const unsigned mp_items[]={57,55,56,47,49,50,46,48,45};
                for(unsigned i=0;i<sizeof mp_items/sizeof mp_items[0];i++)
                    if(jx_inventory_has(&h->c,(uint16_t)mp_items[i])){recovery_item=mp_items[i];break;}
            }
            if(jx_word(&h->c,(uint16_t)(stats+0x10))<400){
                static const unsigned hp_items[]={42,53,46,52,43,54,45,51,48,47,50};
                for(unsigned i=0;i<sizeof hp_items/sizeof hp_items[0];i++)
                    if(jx_inventory_has(&h->c,(uint16_t)hp_items[i])){recovery_item=hp_items[i];break;}
            }
        }
        /* The supply-preserving walkthrough accepts the original battle28
         * defeat branch by using ordinary attacks without recovery items. */
        /* Spend carried tea through the ordinary recovery item menu during
         * the original Niu Gao/Fang Mian two-fight story. No turn or result
         * state is changed by this host keyboard policy. */
        if(actor<5 && jx_word(&h->c,0x9334)==189 &&
           (jx_word(&h->c,0x2934)==25 || jx_word(&h->c,0x2934)==26)){
            uint16_t stats=(uint16_t)(0xd04e + 68*actor);
            unsigned hp_limit=actor==1?500:550;
            unsigned mp_limit=actor==0?150:actor==4?220:100;
            int need_hp=jx_word(&h->c,(uint16_t)(stats+0x10))<hp_limit && !adaptive_can_finish_with_carried_tea(h);
            int need_mp=jx_word(&h->c,(uint16_t)(stats+0x12))<mp_limit;
            if(need_hp || need_mp){
                static const unsigned ordinary_supplies[]={45,43,42,57,47,48,50};
                static const unsigned first_fight_hp[]={43,45,42,47,48,50,57};
                static const unsigned first_fight_mp[]={47,57,43,45,48,50,42};
                const unsigned *supplies=ordinary_supplies;
                if(jx_word(&h->c,0x2934)==25)supplies=need_hp?first_fight_hp:first_fight_mp;
                for(unsigned i=0;i<sizeof ordinary_supplies/sizeof ordinary_supplies[0];i++)
                    if(jx_inventory_has(&h->c,(uint16_t)supplies[i])){recovery_item=supplies[i];break;}
            }
        }
        /* Consume carried supplies via ordinary menus in chapter six onward. */
        if(actor<5 && jx_word(&h->c,0x8894)>=6){
            uint16_t stats=(uint16_t)(0xd04e + 68*actor);
            if(jx_word(&h->c,(uint16_t)(stats+0x12))<220 &&
               jx_word(&h->c,(uint16_t)(stats+0x12))<jx_word(&h->c,(uint16_t)(stats+0x16)) && jx_inventory_has(&h->c,47))recovery_item=47;
            if(jx_word(&h->c,(uint16_t)(stats+0x10))<800 &&
               jx_word(&h->c,(uint16_t)(stats+0x10))<jx_word(&h->c,(uint16_t)(stats+0x14)) && jx_inventory_has(&h->c,45))recovery_item=45;
        }
        if(getenv("JX_MAINLINE_BRANCH") && actor==3 && jx_word(&h->c,0x9334)==175 && jx_word(&h->c,0x2934)==28){
            if(jx_word(&h->c,0xd12a)<600 && jx_inventory_has(&h->c,45))recovery_item=45;
        }
        /* Defeat/load coverage keeps maximum inventory but chooses ordinary
         * attacks without recovery, allowing a genuine loss naturally. */
        if(getenv("JX_VERIFY_DEATH_LOAD"))recovery_item=0;
        int recover=recovery_item!=0;
        if(recover && o==0x80fa && n==5 && a[2]==4){
            unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
            h->adaptive_skill_row=2;h->adaptive_skill_column=0;h->menu_probe=5;h->input_probe_step=0;
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);return result;
        }
        if(recover && o==0x9182 && n==0){
            unsigned prior_menu=h->menu_probe;
            key_scan(h,0x9c);h->menu_probe=0;h->token_inventory_target=recovery_item;
            h->token_inventory_active=1;h->token_inventory_polls=0;h->token_inventory_scan=0;
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->token_inventory_active=0;h->menu_probe=prior_menu;
            if((uint16_t)result!=recovery_item)fail(h,"normal recovery inventory selected an unexpected item");
            printf("native adaptive recovery item selected: actor%u item%u polls=%u\n",actor,recovery_item,h->token_inventory_polls);
            return result;
        }
    }
    if(adaptive_tactics && h->adaptive_choice && jx_word(&h->c,0x9334)==154 && s==0x1f2a && o==0x7e0e && n==8 &&
       a[3]==h->c.data_segment && (a[2]==0x1160 || a[2]==0x1181 || a[2]==0x11a2)){
        /* Wang Zuo's three passwords, selected through original Down/Enter. */
        unsigned expected=a[2]==0x11a2?0:1;
        unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
        h->menu_probe=expected?2:1;h->input_probe_step=0;key_scan(h,0x9c);
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);
        if((uint16_t)result!=expected)fail(h,"normal password menu selected an unexpected answer");
        printf("native adaptive password choice: text=%04x choice=%u\n",a[2],expected);
        return result;
    }
    if(((h->he_mei_proposal_walkthrough && h->walkthrough_phase==66) || (h->adaptive_walkthrough && h->walkthrough_phase>=92 && h->adaptive_choice)) && s==0x1f2a && o==0x7baa && n==6){
        unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
        h->menu_probe=2;h->input_probe_step=0;key_scan(h,0x9c);
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);
        if((uint16_t)result!=1)fail(h,"normal proposal menu did not select continued mainquest");
        printf("native He proposal menu selected: choice1\n");
        return result;
    }
    if(((h->island_rescue_walkthrough && h->walkthrough_phase==55 && jx_word(&h->c,0x2934)==5) || (h->city_wall_zhang_walkthrough && h->walkthrough_phase==70 && jx_word(&h->c,0x2934)==6) || (h->adaptive_walkthrough && h->walkthrough_phase>=92 && h->adaptive_skill)) && s==0x1f2a && h->battle_input && jx_word(&h->c,0xacea)){
        if(o==0x80fa && n==5 && a[2]==4 &&
           (adaptive_tactics?adaptive_best_skill(h,jx_word(&h->c,jx_word(&h->c,0x2930)==0?0x2936:0x2938))>=0:jx_word(&h->c,0xd060)>=20)){
            unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
            h->menu_probe=4;h->input_probe_step=0;
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);
            return result;
        }
        if(o==0x9952 && n==1){
            unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
            int slot=adaptive_tactics?adaptive_best_skill(h,a[0]):2;
            if(slot<0)fail(h,"adaptive skill choice unavailable");
            unsigned expected=adaptive_tactics?jx_ds(&h->c)[(uint16_t)(0xd082+68*a[0]+slot)]:11;
            h->adaptive_skill_row=(unsigned)slot/2;h->adaptive_skill_column=(unsigned)slot%2;
            h->menu_probe=adaptive_tactics?5:2;h->input_probe_step=0;key_scan(h,0x9c);
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);
            if((uint16_t)result!=expected)fail(h,"normal skill menu did not select requested learned skill");
            if(h->adaptive_walkthrough && h->walkthrough_phase>=92)printf("native adaptive skill selected: actor=%u item%u mp=%u\n",a[0],expected,jx_word(&h->c,(uint16_t)(0xd060+68*a[0])));
            else printf("native %s skill selected: item11 mp=%u\n",h->walkthrough_phase==70?"city wall":"rescue",jx_word(&h->c,0xd060));
            return result;
        }
    }
    /* Adaptive give commands select the ordinary party item and inventory
       menus with keyboard input. They never invoke the item callback directly. */
    if(((h->token_guard_walkthrough && h->walkthrough_phase==29) || (h->jia_cup_walkthrough && h->walkthrough_phase==42) || (adaptive_tactics && (h->token_inventory_active || (h->adaptive_end_action>=3 && !jx_word(&h->c,0xacea))))) && s==0x1f2a){
        uint16_t quest_item=adaptive_tactics && h->adaptive_end_action>=3?(uint16_t)(h->adaptive_end_action-3):h->walkthrough_phase==42?19:20;
        if(o==0x80fa && n==5){
            if(a[2]!=3)fail(h,"unexpected token action menu");
            if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
            h->walkthrough_scan=0;
            unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step,prior_story=h->story_probe,prior_battle=h->battle_input;
            h->menu_probe=2;h->input_probe_step=0;h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->menu_probe=prior_menu;h->input_probe_step=prior_step;h->story_probe=prior_story;h->battle_input=prior_battle;key_scan(h,0x9c);
            return result;
        }
        if(o==0x9182 && n==0){
            h->token_inventory_target=quest_item;
            key_scan(h,0x9c);h->menu_probe=0;h->token_inventory_active=1;h->token_inventory_polls=0;h->token_inventory_scan=0;
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->token_inventory_active=0;
            if((uint16_t)result!=quest_item){fprintf(stderr,"quest item menu returned=%u polls=%u cursor=(%u,%u,%u)\n",(unsigned)(uint16_t)result,h->token_inventory_polls,h->token_inventory_column,h->token_inventory_row,h->token_inventory_scroll);export_state(h,"main_decompiled/build/native_token_inventory_failure.bin");fail(h,"inventory input did not select token");}
            if(adaptive_tactics && h->adaptive_end_action>=3)printf("native adaptive gift inventory selected: item%u polls=%u\n",quest_item,h->token_inventory_polls);
            else if(quest_item==20)printf("native token inventory selected: item20 polls=%u\n",h->token_inventory_polls);
            else printf("native cup inventory selected: item19 polls=%u\n",h->token_inventory_polls);
            return result;
        }
        if(o==0x8f78 && n==3 && h->token_inventory_active){
            h->token_inventory_column=a[0];h->token_inventory_row=a[1];h->token_inventory_scroll=a[2];
        }
        if(o==0xc1f2 && n==2 && h->token_inventory_active){
            if(++h->token_inventory_polls>256)fail(h,"token inventory navigation exhausted");
            if(jx_word(&h->c,0x0864)!=jx_word(&h->c,0x0866))return h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            if(h->token_inventory_scan){key_scan(h,(uint8_t)(h->token_inventory_scan|0x80));h->token_inventory_scan=0;}
            else {
                unsigned slot=0;while(slot<64 && jx_word(&h->c,(uint16_t)(0x878a+4*slot))!=h->token_inventory_target)++slot;
                if(slot==64)fail(h,"token disappeared during inventory menu");
                unsigned row=h->token_inventory_row+h->token_inventory_scroll;
                uint8_t scan=h->token_inventory_column!=(slot%2)?0x4d:row<(slot/2)?0x50:row>(slot/2)?0x48:0x1c;
                key_scan(h,scan);h->token_inventory_scan=scan;
            }
            return h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        }
        if(o==0x799a && n==1 && h->walkthrough_phase==42){
            if(a[0]!=19)fail(h,"unexpected cup presentation item");
            ++h->jia_cup_presentations;
        }
        if(o==0xdfec && n==1 && h->walkthrough_phase==42){
            if(a[0]!=9)fail(h,"unexpected cup recipient");
            ++h->jia_cup_callbacks;
        }
        if(o==0xe018 && n==1){
            if(a[0]<18 || a[0]>21)fail(h,"unexpected token recipient");
            ++h->token_guard_callbacks;
        }
    }
    if(h->casino_token_walkthrough && h->walkthrough_phase==27 && s==0x1f2a && o==0x7baa && n==6 && a[2]==0x0f1a){
        unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
        h->menu_probe=2;h->input_probe_step=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);
        return result;
    }
    if(h->casino_token_walkthrough && h->walkthrough_phase==27 && s==0x1f2a && o==0xb6a0 && n==3){
        ++h->casino_token_gambles;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->casino_token_gamble_result=(unsigned)(uint16_t)result;
        printf("native natural casino gamble: result=%u money=%u input_polls=%u\n",h->casino_token_gamble_result,jx_word(&h->c,0x8756),h->story_poll_steps);
        return result;
    }
    if(h->casino_token_walkthrough && h->walkthrough_phase==27 && s==0x1f2a && o==0x4934 && n==3){
        if(a[0]!=4)fail(h,"unexpected casino token battle");
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        uint8_t held=h->walkthrough_scan;
        if(held)key_scan(h,(uint8_t)(held|0x80));
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;++h->casino_token_battles;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->casino_token_battle_result=(unsigned)(uint16_t)result;
        printf("native natural casino battle: battle_id=4 result=%u input_polls=%u\n",h->casino_token_battle_result,h->story_poll_steps);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if((uint16_t)result==1)fail(h,"natural casino battle lost; progression requires recovery");
        if(held)key_scan(h,held);
        return result;
    }
    if(h->city_wall_zhang_walkthrough && h->walkthrough_phase==70 && s==0x1f2a && o==0x4934 && n==3){
        if(a[0]!=6)fail(h,"unexpected city wall encounter");
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        uint8_t held=h->walkthrough_scan;
        if(held)key_scan(h,(uint8_t)(held|0x80));
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        printf("native natural city wall battle: battle_id=6 result=%u input_polls=%u\n",(unsigned)(uint16_t)result,h->story_poll_steps);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if((uint16_t)result==1)fail(h,"natural city wall battle lost");
        if(held)key_scan(h,held);
        return result;
    }
    if(h->island_rescue_walkthrough && h->walkthrough_phase==55 && s==0x1f2a && o==0x4934 && n==3){
        if(a[0]!=5)fail(h,"unexpected island rescue battle");
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        uint8_t held=h->walkthrough_scan;
        if(held)key_scan(h,(uint8_t)(held|0x80));
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;++h->island_rescue_battles;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->island_rescue_result=(unsigned)(uint16_t)result;
        printf("native natural island rescue battle: battle_id=5 result=%u input_polls=%u\n",h->island_rescue_result,h->story_poll_steps);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if((uint16_t)result==1)fail(h,"natural island rescue battle lost; progression requires recovery");
        if(held)key_scan(h,held);
        return result;
    }
    if(h->island_killer_walkthrough && (h->walkthrough_phase>=48 && h->walkthrough_phase<=52) && s==0x1f2a && o==0x4934 && n==3){
        if(a[0]!=2)fail(h,"unexpected island killer battle");
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        uint8_t held=h->walkthrough_scan;
        if(held)key_scan(h,(uint8_t)(held|0x80));
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;++h->island_killer_battles;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->island_killer_result=(unsigned)(uint16_t)result;
        printf("native natural island battle: battle_id=2 result=%u input_polls=%u\n",h->island_killer_result,h->story_poll_steps);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if((uint16_t)result==1)fail(h,"natural island killer battle lost; progression requires recovery");
        if(held)key_scan(h,held);
        return result;
    }
    if(h->tavern_story_walkthrough && h->walkthrough_phase==23 && s==0x1f2a && o==0x4934 && n==3){
        if(a[0]!=3)fail(h,"unexpected tavern story battle");
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        uint8_t held=h->walkthrough_scan;
        if(held)key_scan(h,(uint8_t)(held|0x80));
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;++h->tavern_story_battles;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->tavern_story_battle_result=(unsigned)(uint16_t)result;
        printf("native natural tavern battle: battle_id=3 result=%u input_polls=%u\n",h->tavern_story_battle_result,h->story_poll_steps);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if((uint16_t)result==1)fail(h,"natural tavern story battle lost; progression requires recovery");
        if(held)key_scan(h,held);
        return result;
    }
    if(h->road_walkthrough && ((h->walkthrough_phase>=10 && h->walkthrough_phase<=16) || (h->walkthrough_phase>=78 && h->walkthrough_phase<=89) || (h->adaptive_walkthrough && h->walkthrough_phase>=92)) && s==0x1f2a && o==0x4934 && n==3){
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        uint8_t held=h->walkthrough_scan;
        if(held)key_scan(h,(uint8_t)(held|0x80));
        int caihong_branch=adaptive_tactics && a[0]==28 && a[1]==3 && a[2]==0xffff &&
            jx_word(&h->c,0x9334)==175 && jx_word(&h->c,0x875a)==3 && jx_ds(&h->c)[0x7adc]==1;
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        printf("native natural road encounter: battle_id=%u result=%u input_polls=%u\n",a[0],(unsigned)(uint16_t)result,h->story_poll_steps);
        if(caihong_branch){
            if((uint16_t)result==0xffff && (jx_word(&h->c,0x2932)!=10 || jx_word(&h->c,0xacea)!=0))fail(h,"Caihong ten-turn branch inconsistent");
            if((uint16_t)result==1 && jx_signed_word(jx_word(&h->c,0xd12a))>0)fail(h,"Caihong loss without actual defeat");
            printf("native adaptive caihong original branch: actor=3 map=175 turns=%u result=%u\n",jx_word(&h->c,0x2932),(unsigned)(uint16_t)result);
        }
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if((uint16_t)result==1 && !caihong_branch){
            if(adaptive_tactics)export_state(h,"main_decompiled/build/native_adaptive_battle_failure.bin");
            /* An explicit defeat/load test must let the original caller
             * receive its genuine losing result and open its load menu. */
            if(!(h->adaptive_walkthrough && getenv("JX_VERIFY_DEATH_LOAD")))
                fail(h,"natural road encounter lost; progression requires recovery");
        }
        if(held)key_scan(h,held);
        return result;
    }
    if(h->blood_walkthrough && h->walkthrough_phase==8 && s==0x1f2a && o==0x4934 && n==3){
        if(a[0]!=1)fail(h,"unexpected natural blood quest battle");
        ++h->walkthrough_battles;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->walkthrough_battle_result=(unsigned)(uint16_t)result;
        return result;
    }
    if(adaptive_scene_trigger || adaptive_scene_entry || adaptive_scene_contact){
        /* Ending entry must remain observable after a long saved-game route. */
        int diagnostic=h->walkthrough_phase>=190 &&
            (h->adaptive_diagnostic_calls++<20 || (s==0x2f28 && o==0x2aee));
        if(diagnostic){fprintf(stderr,"adaptive callback begin %04X:%04X map=%u position=(%u,%u) phase=%u\n",s,o,jx_word(&h->c,0x9334),jx_word(&h->c,0x8760),jx_word(&h->c,0x8762),h->walkthrough_phase);export_state(h,"main_decompiled/build/native_adaptive_callback_observation.bin");}
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        h->story_probe=1;h->battle_input=1;if(!prior_story)h->story_poll_steps=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        if(diagnostic)fprintf(stderr,"adaptive callback end %04X:%04X map=%u position=(%u,%u)\n",s,o,jx_word(&h->c,0x9334),jx_word(&h->c,0x8760),jx_word(&h->c,0x8762));
        return result;
    }
    if(h->walkthrough_probe && h->entry_world && s==0x1f2a &&
       ((o==0x7606 && n==0) || (adaptive_tactics && h->adaptive_end_action>=3 && o==0x799a && n==1) || (h->adaptive_walkthrough && h->walkthrough_phase>=92 && (o==0xe702 || o==0xe796)) || (h->hall_walkthrough && h->walkthrough_phase==3 && o==0xddcc) || (h->return_walkthrough && h->walkthrough_phase==7 && o==0xde00) || (h->linan_gate_walkthrough && h->walkthrough_phase==17 && o==0xdfa2) || (((h->brothel_walkthrough && h->walkthrough_phase==19) || (h->nangong_entry_walkthrough && h->walkthrough_phase==38)) && o==0xe02e) || (h->tavern_story_walkthrough && h->walkthrough_phase==23 && o==0xe43c) || (h->island_killer_walkthrough && (h->walkthrough_phase>=48 && h->walkthrough_phase<=52) && o==0xe4ca) || (h->island_clearance_walkthrough && h->walkthrough_phase==53 && o==0xe4e0) || (h->island_rescue_walkthrough && h->walkthrough_phase==55 && o==0xe5d8) || (h->island_zhang_walkthrough && h->walkthrough_phase==56 && o==0xe59e) || (h->island_he_mei_walkthrough && h->walkthrough_phase==59 && o==0xe588) || (h->village_reunion_walkthrough && h->walkthrough_phase==63 && o==0xe350) || (h->city_wall_zhang_walkthrough && h->walkthrough_phase==70 && o==0xe662) || (h->city_wall_reunion_walkthrough && h->walkthrough_phase==75 && o==0xe648) || (h->wuyi_killer_first_walkthrough && h->walkthrough_phase>=80 && h->walkthrough_phase<=89 && (o==0xe17c || o==0xe192 || o==0xe1bc)))){
        unsigned prior_story=h->story_probe,prior_battle=h->battle_input;
        if(h->walkthrough_phase==23 && o==0xe43c){
            if(n!=1 || a[0]!=2)fail(h,"unexpected tavern story contact");
            ++h->tavern_story_contacts;
        }
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->story_probe=prior_story;h->battle_input=prior_battle;
        return result;
    }
    if(h->contact_seg==s && h->contact_off==o){
        if(h->contact_walk && (jx_word(&h->c,0x8760)!=h->contact_expected_x || jx_word(&h->c,0x8762)!=h->contact_expected_y))fail(h,"contact occurred without expected player movement");
        ++h->contact_hits;
        if(h->scheduled_contact){
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            key_scan(h,h->contact_release);
            return result;
        }
    }
    if(h->interactive_entry && s==0x1f2a && o==0x6b38 && n==0){
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->entry_world=1;h->entry_start=jx_word(&h->c,0x585e);
        return result;
    }
    if(h->interactive_entry && s==0x1f2a && o==0xc1f2 && n==2){
        if(h->entry_world && h->interactive_limit &&
            (uint16_t)(jx_word(&h->c,0x585e)-h->entry_start)>=h->interactive_limit)window_shutdown(h);
        if(!jx_native_window_pump(&h->window))window_shutdown(h);
    }
    if(h->original_entry_probe && s==0x1f2a && o==0x6b38 && n==0){
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->story_probe=0;h->battle_input=0;h->original_init_probe=0;
        export_state(h,"main_decompiled/build/native_original_entry_initialized.bin");
        h->entry_world=1;h->entry_start=jx_word(&h->c,0x585e);
        if(h->walkthrough_probe){
            export_state(h,"main_decompiled/build/native_walkthrough_initial.bin");
            printf("native continuous walkthrough START: map=%u position=(%u,%u) NPC2state=%u\n",jx_word(&h->c,0x9334),jx_word(&h->c,0x8760),jx_word(&h->c,0x8762),jx_ds(&h->c)[0xb688]);
        }
        if(h->window_probe)h->interactive_entry=1;
        return result;
    }
    if(h->adaptive_walkthrough && h->adaptive_death_loaded && s==0x1f2a && o==0xc1f2 && n==2 &&
       a[0]==82 && a[1]==0x9600 && jx_word(&h->c,0x0864)==jx_word(&h->c,0x0866)){
        export_state(h,"main_decompiled/build/native_adaptive_death_world_return.bin");
        printf("native adaptive death world return: name=%s phase=%u map=%u position=(%u,%u)\n",h->adaptive_name,h->walkthrough_phase,jx_word(&h->c,0x9334),jx_word(&h->c,0x8760),jx_word(&h->c,0x8762));fflush(stdout);
        longjmp(h->stop,3);
    }
    if(h->walkthrough_probe && h->entry_world && s==0x1f2a && o==0xc1f2 && n==2 &&
       (a[0]!=82 || a[1]!=0x9600) && jx_word(&h->c,0x0864)==jx_word(&h->c,0x0866))++h->walkthrough_nested_empty_polls;
    /* AE40 owns exactly frame+82. Nested battle/menu polls must never
     * advance the walkthrough's world-input controller. */
    if(h->original_entry_probe && h->entry_world && s==0x1f2a && o==0xc1f2 && n==2 && a[0]==82 && a[1]==0x9600 &&
        jx_word(&h->c,0x0864)==jx_word(&h->c,0x0866)){
        if(h->window_probe){
            if(!jx_native_window_pump(&h->window))fail(h,"original entry window fixture closed");
            h->entry_ticks=(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start);
        }
        if(h->walkthrough_probe){
            uint16_t map=jx_word(&h->c,0x9334),x=jx_word(&h->c,0x8760),y=jx_word(&h->c,0x8762);
            if(h->temple_walkthrough && h->walkthrough_phase==2 && map==1){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_temple_walkthrough_final.bin");
                printf("native continuous temple walkthrough PASS: timer_ticks=%u world_frames=%u map=1 position=(%u,%u) tree_state=%u master_present=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_ds(&h->c)[0xb68a],jx_ds(&h->c)[0x6678]);
                if(!h->hall_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=3;h->walkthrough_waypoint=0;
            }
            if(h->hall_walkthrough && h->walkthrough_phase==3 && jx_ds(&h->c)[0xb68a]==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_hall_walkthrough_final.bin");
                printf("native continuous master event PASS: timer_ticks=%u world_frames=%u position=(%u,%u) tree_state=%u master_present=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_ds(&h->c)[0xb68a],jx_ds(&h->c)[0x6678]);
                if(!h->disciple_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=4;h->walkthrough_waypoint=0;
            }
            if(h->return_walkthrough && h->walkthrough_phase==7 && map==0){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_return_walkthrough_final.bin");
                printf("native continuous return walkthrough PASS: timer_ticks=%u world_frames=%u map=0 position=(%u,%u) tree_state=%u permission=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_ds(&h->c)[0xb68a],jx_ds(&h->c)[0x3466]);
                if(!h->blood_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=8;h->walkthrough_waypoint=0;
            }
            if(h->blood_walkthrough && h->walkthrough_phase==8 && jx_ds(&h->c)[0x3469]){
                if(h->walkthrough_battles!=1 || h->walkthrough_battle_result==1 || !jx_inventory_has(&h->c,18))fail(h,"natural blood quest battle/item verification failed");
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_blood_walkthrough_final.bin");
                printf("native continuous blood quest PASS: timer_ticks=%u world_frames=%u position=(%u,%u) east_permission=%u corpses=(%u,%u,%u,%u)\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_ds(&h->c)[0x3469],jx_ds(&h->c)[0x665c],jx_ds(&h->c)[0x665d],jx_ds(&h->c)[0x665e],jx_ds(&h->c)[0x665f]);
                printf("native natural blood evidence: battle_calls=%u battle_id=1 battle_result=%u item18=%u story_input_polls=%u\n",h->walkthrough_battles,h->walkthrough_battle_result,jx_inventory_has(&h->c,18),h->story_poll_steps);
                printf("native nested input isolation: nested_empty_polls=%u\n",h->walkthrough_nested_empty_polls);
                if(!h->east_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=9;h->walkthrough_waypoint=0;
            }
            if(h->east_walkthrough && h->walkthrough_phase==9 && map==3){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_east_walkthrough_final.bin");
                printf("native continuous east exit PASS: timer_ticks=%u world_frames=%u map=3 position=(%u,%u) item18=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_inventory_has(&h->c,18));
                if(!h->road_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=10;h->walkthrough_waypoint=0;
            }
            if(h->road_walkthrough && h->walkthrough_phase==10 && map==4){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_road_walkthrough_final.bin");
                printf("native continuous road walkthrough PASS: timer_ticks=%u world_frames=%u map=4 position=(%u,%u) item18=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_inventory_has(&h->c,18));
                if(!h->wuyi_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=11;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_walkthrough && h->walkthrough_phase==11 && map==6){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_wuyi_walkthrough_final.bin");
                printf("native continuous Wuyi entry PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_inventory_has(&h->c,18));
                if(!h->liu_house_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=12;h->walkthrough_waypoint=0;
            }
            if(h->liu_house_walkthrough && h->walkthrough_phase==12 && map==24){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                export_state(h,"main_decompiled/build/native_liu_house_final.bin");
                printf("native continuous Liu house entry PASS: timer_ticks=%u world_frames=%u map=24 position=(%u,%u) item18=%u ZhangLin_present=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_inventory_has(&h->c,18),jx_ds(&h->c)[0x692c]);
                if(!h->zhang_lin_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=13;h->walkthrough_waypoint=0;
            }
            if(h->zhang_lin_walkthrough && h->walkthrough_phase==13 && jx_ds(&h->c)[0xb958]==1){
                if(map!=24 || !jx_ds(&h->c)[0x692c] || jx_ds(&h->c)[0x66f3] || jx_ds(&h->c)[0x66f4] || !jx_ds(&h->c)[0x6700] || !jx_ds(&h->c)[0x6701] || !jx_inventory_has(&h->c,18))fail(h,"Zhang Lin dialogue did not enable Linan quest");
                export_state(h,"main_decompiled/build/native_zhang_lin_final.bin");
                printf("native continuous Zhang Lin dialogue PASS: timer_ticks=%u world_frames=%u map=24 position=(%u,%u) item18=1 dialogue_state=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->liu_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=14;h->walkthrough_waypoint=0;
            }
            if(h->liu_return_walkthrough && h->walkthrough_phase==14 && map==6){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb958]!=1)fail(h,"Liu house departure lost quest state");
                export_state(h,"main_decompiled/build/native_liu_return_final.bin");
                printf("native continuous Liu return PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->wuyi_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=15;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_return_walkthrough && h->walkthrough_phase==15 && map==4){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb958]!=1)fail(h,"Wuyi return lost quest state");
                export_state(h,"main_decompiled/build/native_wuyi_return_final.bin");
                printf("native continuous Wuyi return PASS: timer_ticks=%u world_frames=%u map=4 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->linan_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=16;h->walkthrough_waypoint=0;
            }
            if(h->linan_walkthrough && h->walkthrough_phase==16 && map==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb958]!=1)fail(h,"Linan arrival lost quest state");
                export_state(h,"main_decompiled/build/native_linan_final.bin");
                printf("native continuous Linan entry PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->linan_gate_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=17;h->walkthrough_waypoint=0;
            }
            if(h->linan_gate_walkthrough && h->walkthrough_phase==17 && jx_ds(&h->c)[0xb739]==1){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=5 || !jx_inventory_has(&h->c,18))fail(h,"Linan gate event lost quest state");
                export_state(h,"main_decompiled/build/native_linan_gate_final.bin");
                printf("native continuous Linan gate PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->city_information_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=18;h->walkthrough_waypoint=0;h->city_information_index=0;
            }
            if(h->city_information_walkthrough && h->walkthrough_phase==18){
                unsigned count=(unsigned)(sizeof city_information_routes/sizeof city_information_routes[0]);
                unsigned npc=city_information_routes[h->city_information_index].npc;
                if(jx_ds(&h->c)[0xb71c+npc]>=4 && h->city_information_index+1<count){
                    printf("native city information dialogue complete: npc=%u state=%u input_polls=%u\n",npc,jx_ds(&h->c)[0xb71c+npc],h->story_poll_steps);
                    ++h->city_information_index;h->walkthrough_waypoint=0;
                }
                if(jx_ds(&h->c)[0x67fe] && jx_ds(&h->c)[0x6800] && jx_ds(&h->c)[0x6803]){
                    unsigned total=0;for(unsigned i=0;i<25;i++)total+=jx_ds(&h->c)[0xb71c+i];
                    if(map!=5 || total<40 || !jx_inventory_has(&h->c,18))fail(h,"city information quest did not complete naturally");
                    export_state(h,"main_decompiled/build/native_city_information_final.bin");
                    printf("native continuous city information PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 dialogue_total=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,total);
                    if(!h->brothel_walkthrough && !h->tavern_walkthrough)longjmp(h->stop,3);
                    h->walkthrough_phase=h->tavern_walkthrough?20:19;h->walkthrough_waypoint=0;
                }
            }
            if(h->brothel_walkthrough && h->walkthrough_phase==19 && map==13){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18))fail(h,"brothel arrival lost letter");
                export_state(h,"main_decompiled/build/native_brothel_final.bin");
                printf("native continuous brothel entry PASS: timer_ticks=%u world_frames=%u map=13 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                longjmp(h->stop,3);
            }
            if(h->tavern_walkthrough && h->walkthrough_phase==20 && map==11){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18))fail(h,"tavern arrival lost letter");
                export_state(h,"main_decompiled/build/native_tavern_final.bin");
                printf("native continuous tavern entry PASS: timer_ticks=%u world_frames=%u map=11 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->tavern_clue_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=21;h->walkthrough_waypoint=0;
            }
            if(h->tavern_clue_walkthrough && h->walkthrough_phase==21 && jx_ds(&h->c)[0xb7d0]==1){
                if(map!=11 || !jx_inventory_has(&h->c,18))fail(h,"tavern clue lost quest state");
                export_state(h,"main_decompiled/build/native_tavern_clue_final.bin");
                printf("native continuous tavern clue PASS: timer_ticks=%u world_frames=%u map=11 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->tavern_upstairs_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=22;h->walkthrough_waypoint=0;
            }
            if(h->tavern_upstairs_walkthrough && h->walkthrough_phase==22 && map==14){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x67fe] || !jx_ds(&h->c)[0x6800] || !jx_ds(&h->c)[0x6803])fail(h,"tavern upstairs lost quest actors");
                export_state(h,"main_decompiled/build/native_tavern_upstairs_final.bin");
                printf("native continuous tavern upstairs PASS: timer_ticks=%u world_frames=%u map=14 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->tavern_story_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=23;h->walkthrough_waypoint=0;
            }
            if(h->tavern_story_walkthrough && h->walkthrough_phase==23 && jx_ds(&h->c)[0xb847]==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=14 || h->tavern_story_contacts!=1 || h->tavern_story_battles!=1 || h->tavern_story_battle_result!=0 || !jx_inventory_has(&h->c,18))fail(h,"tavern story did not complete naturally");
                export_state(h,"main_decompiled/build/native_tavern_story_final.bin");
                printf("native continuous tavern story PASS: timer_ticks=%u world_frames=%u map=14 position=(%u,%u) item18=1 battle_calls=%u battle_result=%u input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->tavern_story_battles,h->tavern_story_battle_result,h->story_poll_steps);
                if(!h->tavern_downstairs_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=24;h->walkthrough_waypoint=0;
            }
            if(h->tavern_downstairs_walkthrough && h->walkthrough_phase==24 && map==11){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb847]!=5)fail(h,"tavern departure lost story state");
                export_state(h,"main_decompiled/build/native_tavern_downstairs_final.bin");
                printf("native continuous tavern downstairs PASS: timer_ticks=%u world_frames=%u map=11 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->tavern_departure_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=25;h->walkthrough_waypoint=0;
            }
            if(h->tavern_departure_walkthrough && h->walkthrough_phase==25 && map==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb847]!=5)fail(h,"tavern city return lost story state");
                export_state(h,"main_decompiled/build/native_tavern_departure_final.bin");
                printf("native continuous tavern departure PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->casino_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=26;h->walkthrough_waypoint=0;
            }
            if(h->casino_walkthrough && h->walkthrough_phase==26 && map==9){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb795]!=5)fail(h,"casino arrival lost story state");
                export_state(h,"main_decompiled/build/native_casino_final.bin");
                printf("native continuous casino PASS: timer_ticks=%u world_frames=%u map=9 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->casino_token_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=27;h->walkthrough_waypoint=0;
            }
            if(h->casino_token_walkthrough && h->walkthrough_phase==27 && jx_inventory_has(&h->c,20)){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=9 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x6769] || h->casino_token_battles!=0 || !h->casino_token_gambles || h->casino_token_gamble_result!=1)fail(h,"casino token quest did not complete naturally");
                export_state(h,"main_decompiled/build/native_casino_token_final.bin");
                printf("native continuous casino token PASS: timer_ticks=%u world_frames=%u map=9 position=(%u,%u) item18=1 item20=1 gamble_calls=%u gamble_result=%u input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->casino_token_gambles,h->casino_token_gamble_result,h->story_poll_steps);
                if(!h->casino_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=28;h->walkthrough_waypoint=0;
            }
            if(h->casino_return_walkthrough && h->walkthrough_phase==28 && map==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || !jx_inventory_has(&h->c,20))fail(h,"casino return lost quest items");
                export_state(h,"main_decompiled/build/native_casino_return_final.bin");
                printf("native continuous casino return PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 item20=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->token_guard_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=29;h->walkthrough_waypoint=0;
            }
            if(h->token_guard_walkthrough && h->walkthrough_phase==29 && !jx_ds(&h->c)[0x6702]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=5 || !jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,20) || h->token_guard_callbacks!=1 || jx_ds(&h->c)[0x6703] || jx_ds(&h->c)[0x6704] || jx_ds(&h->c)[0x6705])fail(h,"guard token presentation did not complete naturally");
                export_state(h,"main_decompiled/build/native_token_guard_final.bin");
                printf("native continuous token guard PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 item20=0 guard_callbacks=%u inventory_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->token_guard_callbacks,h->token_inventory_polls);
                if(!h->xiao_lei_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=30;h->walkthrough_waypoint=0;
            }
            if(h->xiao_lei_walkthrough && h->walkthrough_phase==30 && jx_ds(&h->c)[0xb72b]==6){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=5 || !jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,20))fail(h,"Xiao Lei clue lost quest items");
                export_state(h,"main_decompiled/build/native_xiao_lei_final.bin");
                printf("native continuous Xiao Lei PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 state=6 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->fishing_village_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=31;h->walkthrough_waypoint=0;
            }
            if(h->fishing_village_walkthrough && h->walkthrough_phase==31 && map==12){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,20) || jx_ds(&h->c)[0xb72b]!=6)fail(h,"fishing village arrival lost quest progress");
                export_state(h,"main_decompiled/build/native_fishing_village_final.bin");
                printf("native continuous fishing village PASS: timer_ticks=%u world_frames=%u map=12 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->fisherman_clue_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=32;h->walkthrough_waypoint=0;
            }
            if(h->fisherman_clue_walkthrough && h->walkthrough_phase==32 && jx_ds(&h->c)[0xb7ee]==1){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=12 || !jx_inventory_has(&h->c,18))fail(h,"fisherman clue lost quest state");
                export_state(h,"main_decompiled/build/native_fisherman_clue_final.bin");
                printf("native continuous fisherman clue PASS: timer_ticks=%u world_frames=%u map=12 position=(%u,%u) item18=1 state=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->old_wang_house_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=33;h->walkthrough_waypoint=0;
            }
            if(h->old_wang_house_walkthrough && h->walkthrough_phase==33 && map==17){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb7ee]!=1)fail(h,"Old Wang house lost fisherman clue");
                export_state(h,"main_decompiled/build/native_old_wang_house_final.bin");
                printf("native continuous Old Wang house PASS: timer_ticks=%u world_frames=%u map=17 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->old_wang_clue_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=34;h->walkthrough_waypoint=0;
            }
            if(h->old_wang_clue_walkthrough && h->walkthrough_phase==34 && jx_ds(&h->c)[0xb884]==1){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=17 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb725]!=5)fail(h,"Old Wang clue did not enable Jia quest");
                export_state(h,"main_decompiled/build/native_old_wang_clue_final.bin");
                printf("native continuous Old Wang clue PASS: timer_ticks=%u world_frames=%u map=17 position=(%u,%u) item18=1 state=1 jia_state=5 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->old_wang_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=35;h->walkthrough_waypoint=0;
            }
            if(h->old_wang_return_walkthrough && h->walkthrough_phase==35 && map==12){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb725]!=5)fail(h,"Old Wang return lost Jia clue");
                export_state(h,"main_decompiled/build/native_old_wang_return_final.bin");
                printf("native continuous Old Wang return PASS: timer_ticks=%u world_frames=%u map=12 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->village_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=36;h->walkthrough_waypoint=0;
            }
            if(h->village_return_walkthrough && h->walkthrough_phase==36 && map==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb725]!=5)fail(h,"village city return lost Jia clue");
                export_state(h,"main_decompiled/build/native_village_return_final.bin");
                printf("native continuous village return PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->jia_clue_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=37;h->walkthrough_waypoint=0;
            }
            if(h->jia_clue_walkthrough && h->walkthrough_phase==37 && jx_ds(&h->c)[0xb725]==7){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=5 || !jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x67e3])fail(h,"Jia clue did not enable Nangong Caihong");
                export_state(h,"main_decompiled/build/native_jia_clue_final.bin");
                printf("native continuous Jia clue PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 state=7 nangong=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->nangong_entry_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=38;h->walkthrough_waypoint=0;
            }
            if(h->nangong_entry_walkthrough && h->walkthrough_phase==38 && map==13){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb725]!=7 || !jx_ds(&h->c)[0x67e3])fail(h,"Nangong entry lost Jia clue");
                export_state(h,"main_decompiled/build/native_nangong_entry_final.bin");
                printf("native continuous Nangong entry PASS: timer_ticks=%u world_frames=%u map=13 position=(%u,%u) item18=1 nangong=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->nangong_cup_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=39;h->walkthrough_waypoint=0;
                if(h->test_supply_stage && !h->saved_walkthrough){
                    h->walkthrough_phase=900;h->adaptive_ready=0;
                    adaptive_load_route(h);
                }
            }
            if(h->nangong_cup_walkthrough && h->walkthrough_phase==39 && jx_ds(&h->c)[0xb80f]==6){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=13 || !jx_inventory_has(&h->c,18) || !jx_inventory_has(&h->c,19) || !jx_ds(&h->c)[0x67e3])fail(h,"Nangong dialogue did not grant tea cup");
                export_state(h,"main_decompiled/build/native_nangong_cup_final.bin");
                printf("native continuous Nangong cup PASS: timer_ticks=%u world_frames=%u map=13 position=(%u,%u) item18=1 item19=1 state=6 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->brothel_downstairs_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=40;h->walkthrough_waypoint=0;
            }
            if(h->brothel_downstairs_walkthrough && h->walkthrough_phase==40 && map==7){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || !jx_inventory_has(&h->c,19) || jx_ds(&h->c)[0xb80f]!=6)fail(h,"brothel downstairs lost tea cup");
                export_state(h,"main_decompiled/build/native_brothel_downstairs_final.bin");
                printf("native continuous brothel downstairs PASS: timer_ticks=%u world_frames=%u map=7 position=(%u,%u) item18=1 item19=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->brothel_city_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=41;h->walkthrough_waypoint=0;
            }
            if(h->brothel_city_return_walkthrough && h->walkthrough_phase==41 && map==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(!jx_inventory_has(&h->c,18) || !jx_inventory_has(&h->c,19) || jx_ds(&h->c)[0xb725]!=7)fail(h,"brothel city return lost cup quest");
                export_state(h,"main_decompiled/build/native_brothel_city_return_final.bin");
                printf("native continuous brothel city return PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 item19=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y);
                if(!h->jia_cup_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=42;h->walkthrough_waypoint=0;
            }
            if(h->jia_cup_walkthrough && h->walkthrough_phase==42 && jx_ds(&h->c)[0xb884]==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=5 || !jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,19) || h->jia_cup_callbacks!=1 || h->jia_cup_presentations!=1 || jx_ds(&h->c)[0x66f9] || jx_word(&h->c,0x875c)!=6 || jx_ds(&h->c)[0xb7ef]!=4)fail(h,"Jia cup presentation did not enable sailing naturally");
                export_state(h,"main_decompiled/build/native_jia_cup_final.bin");
                printf("native continuous Jia cup PASS: timer_ticks=%u world_frames=%u map=5 position=(%u,%u) item18=1 item19=0 party=6 cup_callbacks=%u inventory_polls=%u input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->jia_cup_callbacks,h->token_inventory_polls,h->story_poll_steps);
                if(!h->sailing_village_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=43;h->walkthrough_waypoint=0;
            }
            if(h->sailing_village_walkthrough && h->walkthrough_phase==43 && map==12){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=0 || y!=12 || !jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,19) || jx_word(&h->c,0x875c)!=6 || jx_ds(&h->c)[0xb884]!=5)fail(h,"sailing village return lost companion or quest");
                export_state(h,"main_decompiled/build/native_sailing_village_final.bin");
                printf("native continuous sailing village PASS: timer_ticks=%u world_frames=%u map=12 position=(0,12) item18=1 party=6\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start));
                if(!h->sailing_house_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=44;h->walkthrough_waypoint=0;
            }
            if(h->sailing_house_walkthrough && h->walkthrough_phase==44 && map==17){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=10 || y!=17 || !jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,19) || jx_word(&h->c,0x875c)!=6 || jx_ds(&h->c)[0xb884]!=5)fail(h,"sailing house arrival lost companion or quest");
                export_state(h,"main_decompiled/build/native_sailing_house_final.bin");
                printf("native continuous sailing house PASS: timer_ticks=%u world_frames=%u map=17 position=(10,17) item18=1 party=6\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start));
                if(!h->sailing_wang_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=45;h->walkthrough_waypoint=0;
            }
            if(h->sailing_wang_walkthrough && h->walkthrough_phase==45 && !jx_ds(&h->c)[0x6858]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=17 || !jx_inventory_has(&h->c,18) || jx_inventory_has(&h->c,19) || jx_word(&h->c,0x875c)!=6 || jx_ds(&h->c)[0xb884]!=5 || !jx_ds(&h->c)[0x67c4])fail(h,"Old Wang did not leave house for pier naturally");
                export_state(h,"main_decompiled/build/native_sailing_wang_final.bin");
                printf("native continuous sailing Wang PASS: timer_ticks=%u world_frames=%u map=17 position=(%u,%u) item18=1 party=6 pier=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->sailing_pier_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=46;h->walkthrough_waypoint=0;
            }
            if(h->sailing_pier_return_walkthrough && h->walkthrough_phase==46 && map==12){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=89 || y!=10 || !jx_inventory_has(&h->c,18) || jx_word(&h->c,0x875c)!=6 || !jx_ds(&h->c)[0x67c4] || jx_ds(&h->c)[0x6858])fail(h,"pier return lost companion or Old Wang");
                export_state(h,"main_decompiled/build/native_sailing_pier_return_final.bin");
                printf("native continuous sailing pier return PASS: timer_ticks=%u world_frames=%u map=12 position=(89,10) item18=1\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start));
                if(!h->island_sailing_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=47;h->walkthrough_waypoint=0;
            }
            if(h->island_sailing_walkthrough && h->walkthrough_phase==47 && map==18){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=41 || y!=84 || !jx_inventory_has(&h->c,18))fail(h,"sailing did not reach Bixia island naturally");
                export_state(h,"main_decompiled/build/native_island_sailing_final.bin");
                printf("native continuous island sailing PASS: timer_ticks=%u world_frames=%u map=18 position=(41,84) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->island_killer_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=48;h->walkthrough_waypoint=0;
            }
            if(h->island_killer_walkthrough && h->walkthrough_phase==48 && !jx_ds(&h->c)[0x6879]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=18 || !jx_inventory_has(&h->c,18) || h->island_killer_battles!=1 || h->island_killer_result!=0)fail(h,"island killer did not fall in actual battle");
                export_state(h,"main_decompiled/build/native_island_killer_final.bin");
                printf("native continuous island killer PASS: timer_ticks=%u world_frames=%u map=18 position=(%u,%u) item18=1 battle_calls=1 battle_result=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_killer_wu_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=49;h->walkthrough_waypoint=0;
            }
            if(h->island_killer_wu_walkthrough && h->walkthrough_phase==49 && !jx_ds(&h->c)[0x687a]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=18 || !jx_inventory_has(&h->c,18) || h->island_killer_battles!=2 || h->island_killer_result!=0)fail(h,"island killer did not fall in actual battle");
                export_state(h,"main_decompiled/build/native_island_killer_wu_final.bin");
                printf("native continuous island killer Wu PASS: timer_ticks=%u world_frames=%u map=18 position=(%u,%u) item18=1 battle_calls=2 battle_result=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_killer_bing_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=50;h->walkthrough_waypoint=0;
            }
            if(h->island_killer_bing_walkthrough && h->walkthrough_phase==50 && !jx_ds(&h->c)[0x6878]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=18 || !jx_inventory_has(&h->c,18) || h->island_killer_battles!=3 || h->island_killer_result!=0)fail(h,"island killer did not fall in actual battle");
                export_state(h,"main_decompiled/build/native_island_killer_bing_final.bin");
                printf("native continuous island killer Bing PASS: timer_ticks=%u world_frames=%u map=18 position=(%u,%u) item18=1 battle_calls=3 battle_result=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_killer_yi_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=51;h->walkthrough_waypoint=0;
            }
            if(h->island_killer_yi_walkthrough && h->walkthrough_phase==51 && !jx_ds(&h->c)[0x6877]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=18 || !jx_inventory_has(&h->c,18) || h->island_killer_battles!=4 || h->island_killer_result!=0)fail(h,"island killer did not fall in actual battle");
                export_state(h,"main_decompiled/build/native_island_killer_yi_final.bin");
                printf("native continuous island killer Yi PASS: timer_ticks=%u world_frames=%u map=18 position=(%u,%u) item18=1 battle_calls=4 battle_result=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_killer_jia_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=52;h->walkthrough_waypoint=0;
            }
            if(h->island_killer_jia_walkthrough && h->walkthrough_phase==52 && !jx_ds(&h->c)[0x6876]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=18 || !jx_inventory_has(&h->c,18) || h->island_killer_battles!=5 || h->island_killer_result!=0)fail(h,"island killer did not fall in actual battle");
                export_state(h,"main_decompiled/build/native_island_killer_jia_final.bin");
                printf("native continuous island killer Jia PASS: timer_ticks=%u world_frames=%u map=18 position=(%u,%u) item18=1 battle_calls=5 battle_result=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_clearance_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=53;h->walkthrough_waypoint=0;
            }
            if(h->island_clearance_walkthrough && h->walkthrough_phase==53 && map==19){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                for(unsigned i=0;i<5;++i)if(jx_ds(&h->c)[0x6876+i])fail(h,"island clearance left a living killer");
                if(x!=69 || y!=18 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x3478])fail(h,"island clearance did not unlock scene naturally");
                export_state(h,"main_decompiled/build/native_island_clearance_final.bin");
                printf("native continuous island clearance PASS: timer_ticks=%u world_frames=%u map=19 position=(69,18) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->island_hideout_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=54;h->walkthrough_waypoint=0;
            }
            if(h->island_hideout_walkthrough && h->walkthrough_phase==54 && map==20){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=59 || y!=119 || !jx_inventory_has(&h->c,18))fail(h,"island hideout entry did not follow actual exit");
                export_state(h,"main_decompiled/build/native_island_hideout_final.bin");
                printf("native continuous island hideout PASS: timer_ticks=%u world_frames=%u map=20 position=(59,119) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->island_rescue_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=55;h->walkthrough_waypoint=0;
            }
            if(h->island_rescue_walkthrough && h->walkthrough_phase==55 && jx_ds(&h->c)[0x3478]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=20 || x!=56 || y!=104 || !jx_inventory_has(&h->c,18) || h->island_rescue_battles!=1 || h->island_rescue_result!=0 || !jx_ds(&h->c)[0x68b5] || jx_ds(&h->c)[0x3479])fail(h,"island rescue did not finish naturally");
                export_state(h,"main_decompiled/build/native_island_rescue_final.bin");
                printf("native continuous island rescue PASS: timer_ticks=%u world_frames=%u map=20 position=(56,104) item18=1 battle_calls=1 battle_result=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->island_zhang_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=56;h->walkthrough_waypoint=0;
            }
            if(h->island_zhang_walkthrough && h->walkthrough_phase==56 && jx_word(&h->c,0x875c)==1){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=20 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x68b6] || !jx_ds(&h->c)[0x3478])fail(h,"Zhang Linxin did not join naturally after rescue");
                export_state(h,"main_decompiled/build/native_island_zhang_final.bin");
                printf("native continuous island Zhang PASS: timer_ticks=%u world_frames=%u map=20 position=(%u,%u) item18=1 party=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_door_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=57;h->walkthrough_waypoint=0;
            }
            if(h->island_door_walkthrough && h->walkthrough_phase==57 && !jx_ds(&h->c)[0x68b7]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=20 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x68b7] || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x3478])fail(h,"rescue door did not open naturally");
                export_state(h,"main_decompiled/build/native_island_door_final.bin");
                printf("native continuous island door PASS: timer_ticks=%u world_frames=%u map=20 position=(%u,%u) item18=1 party=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_cellar_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=58;h->walkthrough_waypoint=0;
            }
            if(h->island_cellar_walkthrough && h->walkthrough_phase==58 && map==20 && x==65 && y==27){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=20 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x68b7] || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x3478])fail(h,"cellar stairs lost companion or door state");
                export_state(h,"main_decompiled/build/native_island_cellar_final.bin");
                printf("native continuous island cellar PASS: timer_ticks=%u world_frames=%u map=20 position=(%u,%u) item18=1 party=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_he_mei_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=59;h->walkthrough_waypoint=0;
            }
            if(h->island_he_mei_walkthrough && h->walkthrough_phase==59 && jx_word(&h->c,0x875e)==5){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=20 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x68b4] || jx_word(&h->c,0x875c)!=1 || jx_ds(&h->c)[0xb8c0]!=5 || jx_ds(&h->c)[0x3478] || !jx_ds(&h->c)[0x3479])fail(h,"He Mei rescue did not join party and enable return naturally");
                export_state(h,"main_decompiled/build/native_island_he_mei_final.bin");
                printf("native continuous island He Mei PASS: timer_ticks=%u world_frames=%u map=20 position=(%u,%u) item18=1 party=1 he_mei=5 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_cellar_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=60;h->walkthrough_waypoint=0;
            }
            if(h->island_cellar_return_walkthrough && h->walkthrough_phase==60 && map==20 && x==5 && y==77){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=20 || jx_word(&h->c,0x875e)!=5 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x68b4] || jx_word(&h->c,0x875c)!=1 || jx_ds(&h->c)[0xb8c0]!=5 || jx_ds(&h->c)[0x3478] || !jx_ds(&h->c)[0x3479])fail(h,"He Mei rescue did not join party and enable return naturally");
                export_state(h,"main_decompiled/build/native_island_cellar_return_final.bin");
                printf("native continuous island cellar return PASS: timer_ticks=%u world_frames=%u map=20 position=(%u,%u) item18=1 party=1 he_mei=5 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_hideout_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=61;h->walkthrough_waypoint=0;
            }
            if(h->island_hideout_return_walkthrough && h->walkthrough_phase==61 && map==19){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=19 || x!=69 || y!=11 || jx_word(&h->c,0x875e)!=5 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x68b4] || jx_word(&h->c,0x875c)!=1 || jx_ds(&h->c)[0xb8c0]!=5 || jx_ds(&h->c)[0x3478] || !jx_ds(&h->c)[0x3479])fail(h,"He Mei rescue did not join party and enable return naturally");
                export_state(h,"main_decompiled/build/native_island_hideout_return_final.bin");
                printf("native continuous island hideout return PASS: timer_ticks=%u world_frames=%u map=19 position=(%u,%u) item18=1 party=1 he_mei=5 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->island_sailing_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=62;h->walkthrough_waypoint=0;
            }
            if(h->island_sailing_return_walkthrough && h->walkthrough_phase==62 && map==12){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=104 || y!=20 || !jx_inventory_has(&h->c,18) || jx_word(&h->c,0x875c)!=0xffff || jx_word(&h->c,0x875e)!=0xffff || jx_ds(&h->c)[0x67c4] || !jx_ds(&h->c)[0x67c5] || !jx_ds(&h->c)[0x6968] || jx_ds(&h->c)[0xb7ee]!=7 || jx_ds(&h->c)[0xb7ef]!=7 || jx_ds(&h->c)[0xb7f2]!=7)fail(h,"sailing rescue return did not finish naturally");
                export_state(h,"main_decompiled/build/native_island_sailing_return_final.bin");
                printf("native continuous island sailing return PASS: timer_ticks=%u world_frames=%u map=12 position=(104,20) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->village_reunion_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=63;h->walkthrough_waypoint=0;
            }
            if(h->village_reunion_walkthrough && h->walkthrough_phase==63 && jx_ds(&h->c)[0x347d]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=12 || !jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0x67c5] || jx_ds(&h->c)[0x69a2] || !jx_ds(&h->c)[0x68d1] || jx_ds(&h->c)[0xb80f]!=5)fail(h,"village Zhang Rumeng reunion did not advance naturally");
                export_state(h,"main_decompiled/build/native_village_reunion_final.bin");
                printf("native continuous village reunion PASS: timer_ticks=%u world_frames=%u map=12 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->rescued_city_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=64;h->walkthrough_waypoint=0;
            }
            if(h->rescued_city_return_walkthrough && h->walkthrough_phase==64 && map==23){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=159 || y!=80 || !jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d] || jx_ds(&h->c)[0x346b] || !jx_ds(&h->c)[0x68d1])fail(h,"rescued city return did not use chapter-enabled exit");
                export_state(h,"main_decompiled/build/native_rescued_city_return_final.bin");
                printf("native continuous rescued city return PASS: timer_ticks=%u world_frames=%u map=23 position=(159,80) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->he_mei_house_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=65;h->walkthrough_waypoint=0;
            }
            if(h->he_mei_house_walkthrough && h->walkthrough_phase==65 && map==26){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=14 || y!=23 || !jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x6968])fail(h,"He Mei house arrival lost rescued character");
                export_state(h,"main_decompiled/build/native_he_mei_house_final.bin");
                printf("native continuous He Mei house PASS: timer_ticks=%u world_frames=%u map=26 position=(14,23) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),h->story_poll_steps);
                if(!h->he_mei_proposal_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=66;h->walkthrough_waypoint=0;
            }
            if(h->he_mei_proposal_walkthrough && h->walkthrough_phase==66 && jx_ds(&h->c)[0xb992]==7){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=26 || !jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x6969])fail(h,"He proposal did not enable thanks naturally");
                export_state(h,"main_decompiled/build/native_he_mei_proposal_final.bin");
                printf("native continuous He Mei proposal PASS: timer_ticks=%u world_frames=%u map=26 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->he_mei_gift_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=67;h->walkthrough_waypoint=0;
            }
            if(h->he_mei_gift_walkthrough && h->walkthrough_phase==67 && jx_ds(&h->c)[0x696a]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=26 || !jx_inventory_has(&h->c,18) || !jx_inventory_has(&h->c,57) || jx_ds(&h->c)[0x6969])fail(h,"He gift chest did not grant herb naturally");
                export_state(h,"main_decompiled/build/native_he_mei_gift_final.bin");
                printf("native continuous He Mei gift PASS: timer_ticks=%u world_frames=%u map=26 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->he_mei_departure_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=68;h->walkthrough_waypoint=0;
            }
            if(h->he_mei_departure_walkthrough && h->walkthrough_phase==68 && map==23){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=46 || y!=72 || !jx_inventory_has(&h->c,57) || !jx_inventory_has(&h->c,18))fail(h,"He house departure lost gifts");
                export_state(h,"main_decompiled/build/native_he_mei_departure_final.bin");
                printf("native continuous He Mei departure PASS: timer_ticks=%u world_frames=%u map=23 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->city_wall_entry_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=69;h->walkthrough_waypoint=0;
            }
            if(h->city_wall_entry_walkthrough && h->walkthrough_phase==69 && map==21){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=63 || y!=29 || !jx_inventory_has(&h->c,57) || !jx_inventory_has(&h->c,18))fail(h,"City wall entry lost gifts");
                export_state(h,"main_decompiled/build/native_city_wall_entry_final.bin");
                printf("native continuous City wall entry PASS: timer_ticks=%u world_frames=%u map=21 position=(%u,%u) item18=1 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->city_wall_zhang_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=70;h->walkthrough_waypoint=0;
            }
            if(h->city_wall_zhang_walkthrough && h->walkthrough_phase==70 && map==15){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=31 || y!=18 || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb7b2]!=7)fail(h,"City wall Zhang battle transition invalid");
                export_state(h,"main_decompiled/build/native_city_wall_zhang_final.bin");
                printf("native continuous City wall Zhang PASS: timer_ticks=%u world_frames=%u map=15 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->inn_downstairs_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=71;h->walkthrough_waypoint=0;
            }
            if(h->inn_downstairs_walkthrough && h->walkthrough_phase==71 && map==10){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=2 || y!=2 || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || jx_ds(&h->c)[0xb7b2]!=7)fail(h,"Inn downstairs transition invalid");
                export_state(h,"main_decompiled/build/native_inn_downstairs_final.bin");
                printf("native continuous Inn downstairs PASS: timer_ticks=%u world_frames=%u map=10 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->inn_owner_release_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=72;h->walkthrough_waypoint=0;
            }
            if(h->inn_owner_release_walkthrough && h->walkthrough_phase==72 && jx_ds(&h->c)[0xb7b2]==4){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=10 || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Inn owner did not release city naturally");
                export_state(h,"main_decompiled/build/native_inn_owner_release_final.bin");
                printf("native continuous Inn owner release PASS: timer_ticks=%u world_frames=%u map=10 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->inn_city_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=73;h->walkthrough_waypoint=0;
            }
            if(h->inn_city_return_walkthrough && h->walkthrough_phase==73 && map==23){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=43 || y!=106 || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Inn city return invalid");
                export_state(h,"main_decompiled/build/native_inn_city_return_final.bin");
                printf("native continuous Inn city return PASS: timer_ticks=%u world_frames=%u map=23 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->city_wall_return_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=74;h->walkthrough_waypoint=0;
            }
            if(h->city_wall_return_walkthrough && h->walkthrough_phase==74 && map==21){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=63 || y!=29 || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"City wall return invalid");
                export_state(h,"main_decompiled/build/native_city_wall_return_final.bin");
                printf("native continuous City wall return PASS: timer_ticks=%u world_frames=%u map=21 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->city_wall_reunion_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=75;h->walkthrough_waypoint=0;
            }
            if(h->city_wall_reunion_walkthrough && h->walkthrough_phase==75 && jx_word(&h->c,0x875c)==1 && jx_ds(&h->c)[0x346a]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=21 || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"City wall Lin Xin reunion invalid");
                export_state(h,"main_decompiled/build/native_city_wall_reunion_final.bin");
                printf("native continuous City wall reunion PASS: timer_ticks=%u world_frames=%u map=21 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->city_wall_departure_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=76;h->walkthrough_waypoint=0;
            }
            if(h->city_wall_departure_walkthrough && h->walkthrough_phase==76 && map==23){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=63 || y!=29 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"City wall companion departure invalid");
                export_state(h,"main_decompiled/build/native_city_wall_departure_final.bin");
                printf("native continuous City wall departure PASS: timer_ticks=%u world_frames=%u map=23 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->linan_wuyi_departure_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=77;h->walkthrough_waypoint=0;
            }
            if(h->linan_wuyi_departure_walkthrough && h->walkthrough_phase==77 && map==4){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=19 || y!=2 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Linan Wuyi departure invalid");
                export_state(h,"main_decompiled/build/native_linan_wuyi_departure_final.bin");
                printf("native continuous Linan Wuyi departure PASS: timer_ticks=%u world_frames=%u map=4 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_second_entrance_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=78;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_second_entrance_walkthrough && h->walkthrough_phase==78 && map==6){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(x!=0 || y!=112 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Second Wuyi entrance invalid");
                export_state(h,"main_decompiled/build/native_wuyi_second_entrance_final.bin");
                printf("native continuous Second Wuyi entrance PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_second_disciple_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=79;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_second_disciple_walkthrough && h->walkthrough_phase==79 && !jx_ds(&h->c)[0x670f]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Second Wuyi disciple did not yield path");
                export_state(h,"main_decompiled/build/native_wuyi_second_disciple_final.bin");
                printf("native continuous Second Wuyi disciple PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_killer_first_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=80;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_killer_first_walkthrough && h->walkthrough_phase==80 && !jx_ds(&h->c)[0x6719]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi first killer did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_killer_first_final.bin");
                printf("native continuous Wuyi first killer PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_killer_second_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=81;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_killer_second_walkthrough && h->walkthrough_phase==81 && !jx_ds(&h->c)[0x6718]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi second killer did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_killer_second_final.bin");
                printf("native continuous Wuyi second killer PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_killer_third_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=82;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_killer_third_walkthrough && h->walkthrough_phase==82 && !jx_ds(&h->c)[0x6717]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi third killer did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_killer_third_final.bin");
                printf("native continuous Wuyi third killer PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_killer_fourth_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=83;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_killer_fourth_walkthrough && h->walkthrough_phase==83 && !jx_ds(&h->c)[0x6715]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi fourth killer did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_killer_fourth_final.bin");
                printf("native continuous Wuyi fourth killer PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_killer_fifth_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=84;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_killer_fifth_walkthrough && h->walkthrough_phase==84 && !jx_ds(&h->c)[0x6716]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi fifth killer did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_killer_fifth_final.bin");
                printf("native continuous Wuyi fifth killer PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_second_liu_house_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=85;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_second_liu_house_walkthrough && h->walkthrough_phase==85 && map==24){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=24 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi second Liu house did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_second_liu_house_final.bin");
                printf("native continuous Wuyi second Liu house PASS: timer_ticks=%u world_frames=%u map=24 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->liu_wounded_clue_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=86;h->walkthrough_waypoint=0;
            }
            if(h->liu_wounded_clue_walkthrough && h->walkthrough_phase==86 && !jx_ds(&h->c)[0x692a] && jx_ds(&h->c)[0x692b]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=24 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Liu wounded clue did not clear naturally");
                export_state(h,"main_decompiled/build/native_liu_wounded_clue_final.bin");
                printf("native continuous Liu wounded clue PASS: timer_ticks=%u world_frames=%u map=24 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->liu_wounded_departure_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=87;h->walkthrough_waypoint=0;
            }
            if(h->liu_wounded_departure_walkthrough && h->walkthrough_phase==87 && map==6){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=6 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Liu wounded departure did not clear naturally");
                export_state(h,"main_decompiled/build/native_liu_wounded_departure_final.bin");
                printf("native continuous Liu wounded departure PASS: timer_ticks=%u world_frames=%u map=6 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_rear_cave_entry_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=88;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_rear_cave_entry_walkthrough && h->walkthrough_phase==88 && map==25){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=25 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi rear cave entry did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_rear_cave_entry_final.bin");
                printf("native continuous Wuyi rear cave entry PASS: timer_ticks=%u world_frames=%u map=25 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_inner_cave_entry_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=89;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_inner_cave_entry_walkthrough && h->walkthrough_phase==89 && map==22){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=22 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi inner cave entry did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_inner_cave_entry_final.bin");
                printf("native continuous Wuyi inner cave entry PASS: timer_ticks=%u world_frames=%u map=22 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_cave_key_first_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=90;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_cave_key_first_walkthrough && h->walkthrough_phase==90 && !jx_ds(&h->c)[0x68ef]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=22 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi cave first key did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_cave_key_first_final.bin");
                printf("native continuous Wuyi cave first key PASS: timer_ticks=%u world_frames=%u map=22 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->wuyi_cave_key_second_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=91;h->walkthrough_waypoint=0;
            }
            if(h->wuyi_cave_key_second_walkthrough && h->walkthrough_phase==91 && !jx_ds(&h->c)[0x68f1]){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                h->walkthrough_scan=0;
                if(map!=22 || jx_word(&h->c,0x875c)!=1 || !jx_ds(&h->c)[0x346a] || jx_ds(&h->c)[0x68d0] || !jx_inventory_has(&h->c,57) || jx_inventory_has(&h->c,18) || !jx_ds(&h->c)[0x347d])fail(h,"Wuyi cave second key did not clear naturally");
                export_state(h,"main_decompiled/build/native_wuyi_cave_key_second_final.bin");
                printf("native continuous Wuyi cave second key PASS: timer_ticks=%u world_frames=%u map=22 position=(%u,%u) item18=0 input_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                if(!h->adaptive_walkthrough)longjmp(h->stop,3);
                h->walkthrough_phase=92;h->walkthrough_waypoint=0;
            }
            if(h->adaptive_walkthrough && h->walkthrough_phase>=92){
                if(h->adaptive_ready && map==h->adaptive_goal_map &&
                    (h->adaptive_watch_offset==65535 ? (x==h->adaptive_goal_x && y==h->adaptive_goal_y && jx_word(&h->c,0x8764)==h->adaptive_end_facing) : jx_ds(&h->c)[h->adaptive_watch_offset]==h->adaptive_watch_value)){
                    if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));h->walkthrough_scan=0;
                    char path[256];snprintf(path,sizeof path,"main_decompiled/build/native_adaptive_%s_final.bin",h->adaptive_name);export_state(h,path);
                    printf("native adaptive milestone PASS: name=%s phase=%u timer_ticks=%u world_frames=%u map=%u position=(%u,%u) facing=%u party=%u input_polls=%u\n",h->adaptive_name,h->walkthrough_phase,h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),map,x,y,jx_word(&h->c,0x8764),jx_word(&h->c,0x875c),h->story_poll_steps);fflush(stdout);
                    h->adaptive_ready=0;++h->walkthrough_phase;
                }
                if(!h->adaptive_ready)adaptive_load_route(h);
            }
            if(h->walkthrough_phase>=4 && h->walkthrough_phase<=6){
                unsigned state=h->walkthrough_phase==4?0xb6a5:h->walkthrough_phase==5?0xb6a7:0xb6a8;
                if(jx_ds(&h->c)[state]>=4){
                    printf("native disciple dialogue complete: phase=%u state=%u permission=%u\n",h->walkthrough_phase,jx_ds(&h->c)[state],jx_ds(&h->c)[0x3466]);
                    if(h->walkthrough_phase==6){
                        if(!jx_ds(&h->c)[0x3466])fail(h,"disciple dialogue did not enable departure");
                        export_state(h,"main_decompiled/build/native_disciple_walkthrough_final.bin");
                        printf("native continuous disciple walkthrough PASS: timer_ticks=%u world_frames=%u position=(%u,%u) states=(%u,%u,%u) permission=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,jx_ds(&h->c)[0xb6a5],jx_ds(&h->c)[0xb6a7],jx_ds(&h->c)[0xb6a8],jx_ds(&h->c)[0x3466]);
                        if(!h->return_walkthrough)longjmp(h->stop,3);
                    }
                    ++h->walkthrough_phase;h->walkthrough_waypoint=0;
                }
            }
            if(map!=((h->adaptive_walkthrough && h->walkthrough_phase>=92)?h->adaptive_expected_map:h->walkthrough_phase>=90?22:h->walkthrough_phase==89?25:h->walkthrough_phase==88?6:h->walkthrough_phase>=86?24:h->walkthrough_phase>=79?6:h->walkthrough_phase==78?4:h->walkthrough_phase==77?23:h->walkthrough_phase>=75?21:h->walkthrough_phase==74?23:h->walkthrough_phase>=72?10:h->walkthrough_phase==71?15:h->walkthrough_phase==70?21:h->walkthrough_phase==69?23:h->walkthrough_phase>=66?26:h->walkthrough_phase==65?23:h->walkthrough_phase>=63?12:h->walkthrough_phase==62?19:h->walkthrough_phase>=55?20:h->walkthrough_phase==54?19:h->walkthrough_phase>=48?18:h->walkthrough_phase==47?12:h->walkthrough_phase>=45?17:h->walkthrough_phase==44?12:h->walkthrough_phase>=42?5:h->walkthrough_phase==41?7:h->walkthrough_phase>=39?13:h->walkthrough_phase>=37?5:h->walkthrough_phase==36?12:h->walkthrough_phase>=34?17:h->walkthrough_phase>=32?12:h->walkthrough_phase>=29?5:h->walkthrough_phase>=27?9:h->walkthrough_phase==26?5:h->walkthrough_phase==25?11:h->walkthrough_phase>=23?14:h->walkthrough_phase>=21?11:h->walkthrough_phase>=17?5:h->walkthrough_phase==16?4:h->walkthrough_phase==15?6:h->walkthrough_phase>=13?24:h->walkthrough_phase==12?6:h->walkthrough_phase==11?4:h->walkthrough_phase==10?3:(h->walkthrough_phase>=3 && h->walkthrough_phase<=7)?1:0)){ export_state(h,"main_decompiled/build/native_walkthrough_unexpected_map.bin"); fprintf(stderr,"unexpected map=%u position=(%u,%u) facing=%u phase=%u waypoint=%u\n",map,x,y,jx_word(&h->c,0x8764),h->walkthrough_phase,h->walkthrough_waypoint); fail(h,"continuous walkthrough left expected map unexpectedly"); }
            if(jx_ds(&h->c)[0xb688]==1 && h->walkthrough_phase<2){
                if(!h->walkthrough_phase || x!=59 || y!=87)fail(h,"walkthrough quest changed before planned interaction");
                if(h->temple_walkthrough){
                    export_state(h,"main_decompiled/build/native_temple_walkthrough_departure.bin");
                    h->walkthrough_phase=2;h->walkthrough_waypoint=0;
                    printf("native continuous temple departure: timer_ticks=%u position=(%u,%u)\n",h->entry_ticks,x,y);
                }else{
                    export_state(h,"main_decompiled/build/native_walkthrough_final.bin");
                    printf("native continuous walkthrough PASS: timer_ticks=%u world_frames=%u position=(%u,%u) NPC2state=1 dialogue_polls=%u\n",h->entry_ticks,(uint16_t)(jx_word(&h->c,0x585e)-h->entry_start),x,y,h->story_poll_steps);
                    longjmp(h->stop,3);
                }
            }
            if(x!=h->walkthrough_last_x || y!=h->walkthrough_last_y || map!=h->walkthrough_last_map || h->walkthrough_phase!=h->walkthrough_last_phase){
                h->walkthrough_last_x=x;h->walkthrough_last_y=y;h->walkthrough_last_map=map;h->walkthrough_last_phase=h->walkthrough_phase;
                h->walkthrough_last_progress_tick=h->entry_ticks;
            }
            if(h->adaptive_walkthrough && h->walkthrough_phase>=190 && h->entry_ticks%256==0){
                fprintf(stderr,"adaptive observation ticks=%u map=%u position=(%u,%u) facing=%u waypoint=%u/%u direction=%u\n",h->entry_ticks,map,x,y,jx_word(&h->c,0x8764),h->walkthrough_waypoint,h->adaptive_count,jx_word(&h->c,0x2942));
                export_state(h,"main_decompiled/build/native_adaptive_walk_observation.bin");
            }
            if(h->entry_ticks>=131072 || h->entry_ticks-h->walkthrough_last_progress_tick>=128){
                export_state(h,"main_decompiled/build/native_walkthrough_stopped.bin");
                fprintf(stderr,"continuous walkthrough stopped: position=(%u,%u) phase=%u NPC0=(%u,%u)\n",x,y,h->walkthrough_phase,jx_word(&h->c,0x88dc),jx_word(&h->c,0x88de));
                export_state(h,"main_decompiled/build/native_walkthrough_stagnation.bin");
                if(h->test_supply_stage && h->adaptive_walkthrough){
                    if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                    h->walkthrough_scan=0;
                    export_state(h,"main_decompiled/build/native_walkthrough_stagnation.bin");
                    if(h->walkthrough_phase<92){
                        export_state(h,"main_decompiled/build/native_compiled_pause_before_label.bin");
                        unsigned old=h->walkthrough_phase;h->walkthrough_phase+=1000;
                        export_state(h,"main_decompiled/build/native_compiled_pause_after_label.bin");
                        printf("native adaptive compiled prefix paused: old_phase=%u host_phase=%u; guest label exports unchanged\n",old,h->walkthrough_phase);fflush(stdout);
                    }
                    printf("native adaptive route stalled: name=%s phase=%u; guest retained paused\n",h->adaptive_name,h->walkthrough_phase);fflush(stdout);
                    adaptive_load_route(h);
                    h->walkthrough_last_progress_tick=h->entry_ticks;
                    goto adaptive_route_reloaded;
                }
                fail(h,"continuous walkthrough route did not complete");
            }
adaptive_route_reloaded:;
            const uint16_t (*route)[2]=h->walkthrough_phase==2?temple_walkthrough_route:walkthrough_route;
            unsigned count=h->walkthrough_phase==2?(unsigned)(sizeof temple_walkthrough_route/sizeof temple_walkthrough_route[0]):(unsigned)(sizeof walkthrough_route/sizeof walkthrough_route[0]);
            if(h->walkthrough_phase==3){route=hall_walkthrough_route;count=(unsigned)(sizeof hall_walkthrough_route/sizeof hall_walkthrough_route[0]);}
            if(h->walkthrough_phase==4){route=disciple_walkthrough_route;count=(unsigned)(sizeof disciple_walkthrough_route/sizeof disciple_walkthrough_route[0]);}
            if(h->walkthrough_phase==5){route=disciple_second_route;count=(unsigned)(sizeof disciple_second_route/sizeof disciple_second_route[0]);}
            if(h->walkthrough_phase==6){route=disciple_third_route;count=(unsigned)(sizeof disciple_third_route/sizeof disciple_third_route[0]);}
            if(h->walkthrough_phase==7){route=return_walkthrough_route;count=(unsigned)(sizeof return_walkthrough_route/sizeof return_walkthrough_route[0]);}
            if(h->walkthrough_phase==8){route=blood_walkthrough_route;count=(unsigned)(sizeof blood_walkthrough_route/sizeof blood_walkthrough_route[0]);}
            if(h->walkthrough_phase==9){route=east_walkthrough_route;count=(unsigned)(sizeof east_walkthrough_route/sizeof east_walkthrough_route[0]);}
            if(h->walkthrough_phase==10){route=road_walkthrough_route;count=(unsigned)(sizeof road_walkthrough_route/sizeof road_walkthrough_route[0]);}
            if(h->walkthrough_phase==11){route=wuyi_walkthrough_route;count=(unsigned)(sizeof wuyi_walkthrough_route/sizeof wuyi_walkthrough_route[0]);}
            if(h->walkthrough_phase==12){route=liu_house_route;count=(unsigned)(sizeof liu_house_route/sizeof liu_house_route[0]);}
            if(h->walkthrough_phase==13){route=zhang_lin_route;count=(unsigned)(sizeof zhang_lin_route/sizeof zhang_lin_route[0]);}
            if(h->walkthrough_phase==14){route=liu_return_route;count=(unsigned)(sizeof liu_return_route/sizeof liu_return_route[0]);}
            if(h->walkthrough_phase==15){route=wuyi_return_route;count=(unsigned)(sizeof wuyi_return_route/sizeof wuyi_return_route[0]);}
            if(h->walkthrough_phase==16){route=linan_route;count=(unsigned)(sizeof linan_route/sizeof linan_route[0]);}
            if(h->walkthrough_phase==17){route=linan_gate_route;count=(unsigned)(sizeof linan_gate_route/sizeof linan_gate_route[0]);}
            if(h->walkthrough_phase==18){route=city_information_routes[h->city_information_index].route;count=city_information_routes[h->city_information_index].count;}
            if(h->walkthrough_phase==19){route=brothel_route;count=(unsigned)(sizeof brothel_route/sizeof brothel_route[0]);}
            if(h->walkthrough_phase==20){route=tavern_route;count=(unsigned)(sizeof tavern_route/sizeof tavern_route[0]);}
            if(h->walkthrough_phase==21){route=tavern_clue_route;count=(unsigned)(sizeof tavern_clue_route/sizeof tavern_clue_route[0]);}
            if(h->walkthrough_phase==22){route=tavern_upstairs_route;count=(unsigned)(sizeof tavern_upstairs_route/sizeof tavern_upstairs_route[0]);}
            if(h->walkthrough_phase==23){route=tavern_story_route;count=(unsigned)(sizeof tavern_story_route/sizeof tavern_story_route[0]);}
            if(h->walkthrough_phase==24){route=tavern_downstairs_route;count=(unsigned)(sizeof tavern_downstairs_route/sizeof tavern_downstairs_route[0]);}
            if(h->walkthrough_phase==25){route=tavern_departure_route;count=(unsigned)(sizeof tavern_departure_route/sizeof tavern_departure_route[0]);}
            if(h->walkthrough_phase==26){route=casino_route;count=(unsigned)(sizeof casino_route/sizeof casino_route[0]);}
            if(h->walkthrough_phase==27){route=casino_token_route;count=(unsigned)(sizeof casino_token_route/sizeof casino_token_route[0]);}
            if(h->walkthrough_phase==28){route=casino_return_route;count=(unsigned)(sizeof casino_return_route/sizeof casino_return_route[0]);}
            if(h->walkthrough_phase==29){route=token_guard_route;count=(unsigned)(sizeof token_guard_route/sizeof token_guard_route[0]);}
            if(h->walkthrough_phase==30){route=xiao_lei_route;count=(unsigned)(sizeof xiao_lei_route/sizeof xiao_lei_route[0]);}
            if(h->walkthrough_phase==31){route=fishing_village_route;count=(unsigned)(sizeof fishing_village_route/sizeof fishing_village_route[0]);}
            if(h->walkthrough_phase==32){route=fisherman_clue_route;count=(unsigned)(sizeof fisherman_clue_route/sizeof fisherman_clue_route[0]);}
            if(h->walkthrough_phase==33){route=old_wang_house_route;count=(unsigned)(sizeof old_wang_house_route/sizeof old_wang_house_route[0]);}
            if(h->walkthrough_phase==34){route=old_wang_clue_route;count=(unsigned)(sizeof old_wang_clue_route/sizeof old_wang_clue_route[0]);}
            if(h->walkthrough_phase==35){route=old_wang_return_route;count=(unsigned)(sizeof old_wang_return_route/sizeof old_wang_return_route[0]);}
            if(h->walkthrough_phase==36){route=village_return_route;count=(unsigned)(sizeof village_return_route/sizeof village_return_route[0]);}
            if(h->walkthrough_phase==37){route=jia_clue_route;count=(unsigned)(sizeof jia_clue_route/sizeof jia_clue_route[0]);}
            if(h->walkthrough_phase==38){route=nangong_entry_route;count=(unsigned)(sizeof nangong_entry_route/sizeof nangong_entry_route[0]);}
            if(h->walkthrough_phase==39){route=nangong_cup_route;count=(unsigned)(sizeof nangong_cup_route/sizeof nangong_cup_route[0]);}
            if(h->walkthrough_phase==40){route=brothel_downstairs_route;count=(unsigned)(sizeof brothel_downstairs_route/sizeof brothel_downstairs_route[0]);}
            if(h->walkthrough_phase==41){route=brothel_city_return_route;count=(unsigned)(sizeof brothel_city_return_route/sizeof brothel_city_return_route[0]);}
            if(h->walkthrough_phase==83){route=wuyi_killer_fourth_route;count=(unsigned)(sizeof wuyi_killer_fourth_route/sizeof wuyi_killer_fourth_route[0]);}
            if(h->walkthrough_phase==84){route=wuyi_killer_fifth_route;count=(unsigned)(sizeof wuyi_killer_fifth_route/sizeof wuyi_killer_fifth_route[0]);}
            if(h->walkthrough_phase==85){route=wuyi_second_liu_house_route;count=(unsigned)(sizeof wuyi_second_liu_house_route/sizeof wuyi_second_liu_house_route[0]);}
            if(h->walkthrough_phase==86){route=liu_wounded_clue_route;count=(unsigned)(sizeof liu_wounded_clue_route/sizeof liu_wounded_clue_route[0]);}
            if(h->walkthrough_phase==87){route=liu_wounded_departure_route;count=(unsigned)(sizeof liu_wounded_departure_route/sizeof liu_wounded_departure_route[0]);}
            if(h->walkthrough_phase==88){route=wuyi_rear_cave_entry_route;count=(unsigned)(sizeof wuyi_rear_cave_entry_route/sizeof wuyi_rear_cave_entry_route[0]);}
            if(h->walkthrough_phase==89){route=wuyi_inner_cave_entry_route;count=(unsigned)(sizeof wuyi_inner_cave_entry_route/sizeof wuyi_inner_cave_entry_route[0]);}
            if(h->walkthrough_phase==90){route=wuyi_cave_key_first_route;count=(unsigned)(sizeof wuyi_cave_key_first_route/sizeof wuyi_cave_key_first_route[0]);}
            if(h->walkthrough_phase==91){route=wuyi_cave_key_second_route;count=(unsigned)(sizeof wuyi_cave_key_second_route/sizeof wuyi_cave_key_second_route[0]);}
            if(h->walkthrough_phase==82){route=wuyi_killer_third_route;count=(unsigned)(sizeof wuyi_killer_third_route/sizeof wuyi_killer_third_route[0]);}
            if(h->walkthrough_phase==81){route=wuyi_killer_second_route;count=(unsigned)(sizeof wuyi_killer_second_route/sizeof wuyi_killer_second_route[0]);}
            if(h->walkthrough_phase==80){route=wuyi_killer_first_route;count=(unsigned)(sizeof wuyi_killer_first_route/sizeof wuyi_killer_first_route[0]);}
            if(h->walkthrough_phase==79){route=wuyi_second_disciple_route;count=(unsigned)(sizeof wuyi_second_disciple_route/sizeof wuyi_second_disciple_route[0]);}
            if(h->walkthrough_phase==78){route=wuyi_second_entrance_route;count=(unsigned)(sizeof wuyi_second_entrance_route/sizeof wuyi_second_entrance_route[0]);}
            if(h->walkthrough_phase==77){route=linan_wuyi_departure_route;count=(unsigned)(sizeof linan_wuyi_departure_route/sizeof linan_wuyi_departure_route[0]);}
            if(h->walkthrough_phase==76){route=city_wall_departure_route;count=(unsigned)(sizeof city_wall_departure_route/sizeof city_wall_departure_route[0]);}
            if(h->walkthrough_phase==75){route=city_wall_reunion_route;count=(unsigned)(sizeof city_wall_reunion_route/sizeof city_wall_reunion_route[0]);}
            if(h->walkthrough_phase==74){route=city_wall_return_route;count=(unsigned)(sizeof city_wall_return_route/sizeof city_wall_return_route[0]);}
            if(h->walkthrough_phase==73){route=inn_city_return_route;count=(unsigned)(sizeof inn_city_return_route/sizeof inn_city_return_route[0]);}
            if(h->walkthrough_phase==72){route=inn_owner_release_route;count=(unsigned)(sizeof inn_owner_release_route/sizeof inn_owner_release_route[0]);}
            if(h->walkthrough_phase==71){route=inn_downstairs_route;count=(unsigned)(sizeof inn_downstairs_route/sizeof inn_downstairs_route[0]);}
            if(h->walkthrough_phase==70){route=city_wall_zhang_route;count=(unsigned)(sizeof city_wall_zhang_route/sizeof city_wall_zhang_route[0]);}
            if(h->walkthrough_phase==69){route=city_wall_entry_route;count=(unsigned)(sizeof city_wall_entry_route/sizeof city_wall_entry_route[0]);}
            if(h->walkthrough_phase==68){route=he_mei_departure_route;count=(unsigned)(sizeof he_mei_departure_route/sizeof he_mei_departure_route[0]);}
            if(h->walkthrough_phase==67){route=he_mei_gift_route;count=(unsigned)(sizeof he_mei_gift_route/sizeof he_mei_gift_route[0]);}
            if(h->walkthrough_phase==66){route=he_mei_proposal_route;count=(unsigned)(sizeof he_mei_proposal_route/sizeof he_mei_proposal_route[0]);}
            if(h->walkthrough_phase==65){route=he_mei_house_route;count=(unsigned)(sizeof he_mei_house_route/sizeof he_mei_house_route[0]);}
            if(h->walkthrough_phase==64){route=rescued_city_return_route;count=(unsigned)(sizeof rescued_city_return_route/sizeof rescued_city_return_route[0]);}
            if(h->walkthrough_phase==63){route=village_reunion_route;count=(unsigned)(sizeof village_reunion_route/sizeof village_reunion_route[0]);}
            if(h->walkthrough_phase==62){route=island_sailing_return_route;count=(unsigned)(sizeof island_sailing_return_route/sizeof island_sailing_return_route[0]);}
            if(h->walkthrough_phase==61){route=island_hideout_return_route;count=(unsigned)(sizeof island_hideout_return_route/sizeof island_hideout_return_route[0]);}
            if(h->walkthrough_phase==60){route=island_cellar_return_route;count=(unsigned)(sizeof island_cellar_return_route/sizeof island_cellar_return_route[0]);}
            if(h->walkthrough_phase==59){route=island_he_mei_route;count=(unsigned)(sizeof island_he_mei_route/sizeof island_he_mei_route[0]);}
            if(h->walkthrough_phase==58){route=island_cellar_route;count=(unsigned)(sizeof island_cellar_route/sizeof island_cellar_route[0]);}
            if(h->walkthrough_phase==57){route=island_door_route;count=(unsigned)(sizeof island_door_route/sizeof island_door_route[0]);}
            if(h->walkthrough_phase==56){route=island_zhang_route;count=(unsigned)(sizeof island_zhang_route/sizeof island_zhang_route[0]);}
            if(h->walkthrough_phase==55){route=NULL;count=0;}
            if(h->walkthrough_phase==54){route=island_hideout_route;count=(unsigned)(sizeof island_hideout_route/sizeof island_hideout_route[0]);}
            if(h->walkthrough_phase==53){route=island_clearance_route;count=(unsigned)(sizeof island_clearance_route/sizeof island_clearance_route[0]);}
            if(h->walkthrough_phase==52){route=island_killer_jia_route;count=(unsigned)(sizeof island_killer_jia_route/sizeof island_killer_jia_route[0]);}
            if(h->walkthrough_phase==51){route=island_killer_yi_route;count=(unsigned)(sizeof island_killer_yi_route/sizeof island_killer_yi_route[0]);}
            if(h->walkthrough_phase==50){route=island_killer_bing_route;count=(unsigned)(sizeof island_killer_bing_route/sizeof island_killer_bing_route[0]);}
            if(h->walkthrough_phase==49){route=island_killer_wu_route;count=(unsigned)(sizeof island_killer_wu_route/sizeof island_killer_wu_route[0]);}
            if(h->walkthrough_phase==48){route=island_killer_route;count=(unsigned)(sizeof island_killer_route/sizeof island_killer_route[0]);}
            if(h->walkthrough_phase==47){route=island_sailing_route;count=(unsigned)(sizeof island_sailing_route/sizeof island_sailing_route[0]);}
            if(h->walkthrough_phase==46){route=sailing_pier_return_route;count=(unsigned)(sizeof sailing_pier_return_route/sizeof sailing_pier_return_route[0]);}
            if(h->walkthrough_phase==45){route=sailing_wang_route;count=(unsigned)(sizeof sailing_wang_route/sizeof sailing_wang_route[0]);}
            if(h->walkthrough_phase==44){route=sailing_house_route;count=(unsigned)(sizeof sailing_house_route/sizeof sailing_house_route[0]);}
            if(h->walkthrough_phase==43){route=sailing_village_route;count=(unsigned)(sizeof sailing_village_route/sizeof sailing_village_route[0]);}
            if(h->walkthrough_phase==42){route=jia_cup_route;count=(unsigned)(sizeof jia_cup_route/sizeof jia_cup_route[0]);}
            if(h->adaptive_walkthrough && h->walkthrough_phase>=92){route=h->adaptive_route;count=h->adaptive_count;}
            while(h->walkthrough_waypoint<count && x==route[h->walkthrough_waypoint][0] && y==route[h->walkthrough_waypoint][1]){
                ++h->walkthrough_waypoint;
                printf("native continuous walkthrough waypoint=%u/%u position=(%u,%u)\n",h->walkthrough_waypoint,count,x,y);
            }
            uint8_t wanted=0;
            if(h->walkthrough_waypoint<count){
                uint16_t target_x=route[h->walkthrough_waypoint][0],target_y=route[h->walkthrough_waypoint][1];
                wanted=x<target_x?0x4d:x>target_x?0x4b:y<target_y?0x50:y>target_y?0x48:0;
             }else if(h->adaptive_walkthrough && h->walkthrough_phase>=92){
                static const uint8_t scans[4]={0x48,0x50,0x4b,0x4d};
                unsigned state_base=0xb686+30*map;
                if(h->test_supply_stage && h->walkthrough_phase<900 && h->adaptive_end_action==1 && h->adaptive_goal_map==map &&
                   h->adaptive_watch_offset>=state_base && h->adaptive_watch_offset<state_base+30){
                    unsigned npc=h->adaptive_watch_offset-state_base;
                    uint16_t at=(uint16_t)(0x889c+86*npc),nx=jx_word(&h->c,(uint16_t)(at+64)),ny=jx_word(&h->c,(uint16_t)(at+66));
                    uint16_t facing=jx_word(&h->c,0x8764),fx=x,fy=y;
                    if(facing==0)--fy;else if(facing==1)++fy;else if(facing==2)--fx;else ++fx;
                    if(fx+1<nx || fx>nx+1 || fy+1<ny || fy>ny+1)
                        wanted=x<nx?0x4d:x>nx?0x4b:y<ny+2?0x50:y>ny+2?0x48:facing!=0?0x48:0;
                    printf("native adaptive live NPC approach: name=%s npc=%u from=(%u,%u) target=(%u,%u) scan=%u\n",h->adaptive_name,npc,x,y,nx,ny,wanted);
                } else if(jx_signed_word(jx_word(&h->c,0xd0fa))>0 &&
                   (x!=route[count-1][0] || y!=route[count-1][1])) {
                    uint16_t target_x=route[count-1][0],target_y=route[count-1][1];
                    wanted=x<target_x?0x4d:x>target_x?0x4b:y<target_y?0x50:0x48;
                } else if(h->adaptive_end_action==0 || jx_word(&h->c,0x8764)!=h->adaptive_end_facing)wanted=scans[h->adaptive_end_facing];
             }else if(h->walkthrough_phase==79){
                uint16_t nx=jx_word(&h->c,0x8932),ny=jx_word(&h->c,0x8934);
                wanted=x<nx?0x4d:x>nx?0x4b:y<ny+2?0x50:y>ny+2?0x48:jx_word(&h->c,0x8764)!=0?0x48:0;
            }else if(h->walkthrough_phase==37 || h->walkthrough_phase==42 || h->walkthrough_phase==72){
                uint16_t npc=(uint16_t)(0x889c+86*(h->walkthrough_phase==72?0:9)),nx=jx_word(&h->c,(uint16_t)(npc+64)),ny=jx_word(&h->c,(uint16_t)(npc+66));
                uint16_t facing=jx_word(&h->c,0x8764),fx=x,fy=y;
                if(facing==0)--fy;else if(facing==1)++fy;else if(facing==2)--fx;else ++fx;
                if(fx+1<nx || fx>nx+1 || fy+1<ny || fy>ny+1)
                    wanted=x<nx?0x4d:x>nx?0x4b:y<ny+2?0x50:y>ny+2?0x48:facing!=0?0x48:0;
            }else if(h->test_supply_stage && h->walkthrough_phase>=48 && h->walkthrough_phase<=52){
                /* After reaching the planned approach, follow the live patrol
                 * with ordinary keys. Original collision/contact remains in
                 * charge; do not walk past a guard's outdated spawn point. */
                static const unsigned guards[5]={3,4,2,1,0};
                unsigned npc=guards[h->walkthrough_phase-48];
                uint16_t at=(uint16_t)(0x889c+86*npc),nx=jx_word(&h->c,(uint16_t)(at+64)),ny=jx_word(&h->c,(uint16_t)(at+66));
                wanted=x<nx?0x4d:x>nx?0x4b:y<ny?0x50:0x48;
                printf("native test patrol follow: phase=%u npc=%u from=(%u,%u) target=(%u,%u) scan=%u\n",h->walkthrough_phase,npc,x,y,nx,ny,wanted);
            }else if(h->walkthrough_phase==15 || h->walkthrough_phase==22 || h->walkthrough_phase==36 || h->walkthrough_phase==40 || h->walkthrough_phase==48 || h->walkthrough_phase==63 || h->walkthrough_phase==64 || h->walkthrough_phase==76 || h->walkthrough_phase==81 || h->walkthrough_phase==82)wanted=0x4b;
            else if(h->walkthrough_phase==24 || h->walkthrough_phase==31 || h->walkthrough_phase==43 || h->walkthrough_phase==69 || h->walkthrough_phase==71 || h->walkthrough_phase==74 || h->walkthrough_phase==78 || h->walkthrough_phase==83)wanted=0x4d;
            else if(h->walkthrough_phase==26 || h->walkthrough_phase==33 || h->walkthrough_phase==38 || h->walkthrough_phase==44 || h->walkthrough_phase==50 || h->walkthrough_phase==51 || h->walkthrough_phase==52 || h->walkthrough_phase==53 || h->walkthrough_phase==54 || h->walkthrough_phase==55 || h->walkthrough_phase==59 || h->walkthrough_phase==65 || h->walkthrough_phase==80 || h->walkthrough_phase==85 || h->walkthrough_phase==88 || h->walkthrough_phase==89)wanted=0x48;
            else if(h->walkthrough_phase==29 || h->walkthrough_phase==32){if(jx_word(&h->c,0x8764)!=3)wanted=0x4d;}
            else if(h->walkthrough_phase==62){if(jx_word(&h->c,0x8764)!=2)wanted=0x4b;}
            else if(h->walkthrough_phase==56){if(jx_word(&h->c,0x8764)!=3)wanted=0x4d;}
            else if(h->walkthrough_phase==47){if(jx_word(&h->c,0x8764)!=1)wanted=0x50;}
            else if(h->walkthrough_phase==21){if(jx_word(&h->c,0x8764)!=3)wanted=0x4d;}
            else if(h->walkthrough_phase==18){
                unsigned facing=city_information_routes[h->city_information_index].facing;
                if(jx_word(&h->c,0x8764)!=facing)wanted=facing==3?0x4d:0x48;
            }
            else if(h->walkthrough_phase==7 || h->walkthrough_phase==14 || h->walkthrough_phase==25 || h->walkthrough_phase==28 || h->walkthrough_phase==35 || h->walkthrough_phase==41 || h->walkthrough_phase==46 || h->walkthrough_phase==49 || h->walkthrough_phase==58 || h->walkthrough_phase==60 || h->walkthrough_phase==61 || h->walkthrough_phase==68 || h->walkthrough_phase==73 || h->walkthrough_phase==75 || h->walkthrough_phase==77 || h->walkthrough_phase==84 || h->walkthrough_phase==87)wanted=0x50;
            else if(h->walkthrough_phase>=9 && h->walkthrough_phase<=11)wanted=0x4d;
            else if(h->walkthrough_phase==2 || h->walkthrough_phase==3 || h->walkthrough_phase==12 || h->walkthrough_phase==16 || h->walkthrough_phase==17 || h->walkthrough_phase==19 || h->walkthrough_phase==20 || h->walkthrough_phase==23 || jx_word(&h->c,0x8764)!=0)wanted=0x48;
            if(wanted!=h->walkthrough_scan){
                if(h->walkthrough_scan)key_scan(h,(uint8_t)(h->walkthrough_scan|0x80));
                if(wanted)key_scan(h,wanted);
                h->walkthrough_scan=wanted;
            }
            if(!wanted && !h->walkthrough_phase){
                h->walkthrough_phase=1;key_scan(h,0x39);key_scan(h,0xb9);
            }
            if(!wanted && (h->walkthrough_phase==29 || h->walkthrough_phase==42 || (h->adaptive_walkthrough && h->walkthrough_phase>=92 && h->adaptive_end_action>=3))){key_scan(h,0x1c);key_scan(h,0x9c);}
            if(!wanted && ((h->walkthrough_phase>=4 && h->walkthrough_phase<=6) || h->walkthrough_phase==8 || h->walkthrough_phase==13 || h->walkthrough_phase==18 || h->walkthrough_phase==21 || h->walkthrough_phase==27 || h->walkthrough_phase==30 || h->walkthrough_phase==32 || h->walkthrough_phase==34 || h->walkthrough_phase==37 || h->walkthrough_phase==39 || h->walkthrough_phase==45 || h->walkthrough_phase==47 || h->walkthrough_phase==48 || h->walkthrough_phase==56 || h->walkthrough_phase==57 || h->walkthrough_phase==62 || h->walkthrough_phase==66 || h->walkthrough_phase==67 || h->walkthrough_phase==72 || h->walkthrough_phase==79 || h->walkthrough_phase==86 || h->walkthrough_phase==90 || h->walkthrough_phase==91 || (h->adaptive_walkthrough && h->walkthrough_phase>=92 && h->adaptive_end_action==1))){key_scan(h,0x39);key_scan(h,0xb9);}
        }
        if(!h->walkthrough_probe && h->entry_ticks>=64){
            export_state(h,"main_decompiled/build/native_original_entry_final.bin");
            printf("native original B1FA/6B38/AE40 entry PASS: frames=%u polls=%u\n",h->entry_ticks,h->story_poll_steps);
            if(h->window_probe){
                if(!h->window.paints || h->window.ticks<64)fail(h,"original entry window timer/paint fixture incomplete");
                FILE *report=fopen("main_decompiled/analysis/native_original_entry_window_verification.json","wb");
                if(!report)fail(h,"cannot create original entry window report");
                fprintf(report,"{\"status\":\"PASS\",\"world_frames\":%u,\"opening_input_polls\":%u,\"paints\":%u,\"timer_messages\":%u,\"audio_submissions\":%u,\"cd_audio_submissions\":%u,\"scope\":\"Native B1FA/6B38/AE40 original entry, actual window paints/timers and shared interactive event pump; explicit title/opening keyboard fixture, supplied resources and CD/SB PCM device submissions. No full quest playthrough or human audibility asserted.\"}\n",h->entry_ticks,h->story_poll_steps,h->window.paints,h->window.ticks,h->audio.plays,h->cd.audio.plays);
                if(fclose(report))fail(h,"original entry window report close failed");
            }
            longjmp(h->stop,3);
        }
        if(!h->window_probe){timer_tick(h);++h->entry_ticks;}
    }
    if(h->ending_probe && s==0x2f28 && (o==0x3f66 || o==0x415e || o==0x648e || o==0x7062 || o==0x3b8e || o==0x7ec8) && getenv("JX_TRACE_ENDING"))
        printf("ending phase %04x:%04x polls=%u\n",s,o,h->story_poll_steps);
    if(h->original_init_probe && s==0x1f2a && o==0x7baa && n==6){
        h->menu_probe=1;h->input_probe_step=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->menu_probe=0;key_scan(h,0x9c);return result;
    }
    /* Original17D2:01DA contains CD3F 0002 0656: overlay002 board entry. */
    if(s==0x17d2 && o==0x01da && n==1){
        if(h->adaptive_walkthrough && h->entry_world && jx_word(&h->c,0x9334)==194){
            h->adaptive_board_column=9;h->adaptive_board_row=9;
            printf("native adaptive board puzzle begin: resource=%u\n",a[0]);
            int result=jx_board_run(&h->c,(int16_t)a[0],adaptive_board_poll,bios_ticks,h);
            printf("native adaptive board puzzle returned: resource=%u result=%d cursor=(%u,%u)\n",a[0],result,h->adaptive_board_column,h->adaptive_board_row);
            return (uint16_t)result;
        }
        if(h->board_probe_mode){h->memory[0x9fe00]=9;h->memory[0x9fe01]=0;h->memory[0x9fe02]=9;h->memory[0x9fe03]=0;h->memory[0x9fe04]=0;}
        return (uint16_t)jx_board_run(&h->c,(int16_t)a[0],h->board_probe_mode?board_probe_poll:input_poll,bios_ticks,h);
    }
    if(s==0x1832){
        if(o==0x69b3 && n==1 && a[0]==18){
            JxLegacyVideoDevice device={h,legacy_bios};uint16_t result=0;
            if(!jx_legacy_image_enter_mode18(&h->c,&device,legacy_near,legacy_point,h,&result))fail(h,"unsupported legacy mode18 switch path");
            return result;
        }
        if(o==0x6ca3 && n==2)return jx_legacy_video_set_palette(&h->c,(JxFarPointer){a[0],a[1]},legacy_palette,h);
        if(o==0x6e90 && n==5){
            if(jx_legacy_image_put(&h->c,&h->raw,a[0],a[1],(JxFarPointer){a[2],a[3]},a[4],legacy_point,legacy_near,legacy_bank,h)!=1)fail(h,"unsupported legacy image put path");
            return 0;
        }
        if(o==0x6df7 && n==6){
            if(jx_legacy_image_capture(&h->c,&h->raw,a[0],a[1],a[2],a[3],(JxFarPointer){a[4],a[5]},legacy_point,legacy_near,legacy_bank,h)!=1)fail(h,"unsupported legacy image capture path");
            return 0;
        }
    }
    /* Native compatibility policy: supplied assets run without an audio CD.
     * Keep original validation and device errors available in strict mode;
     * do not fabricate successful CD status or nonexistent music. */
    if(((h->scripted_battles && h->story_probe) || (h->adaptive_walkthrough && h->entry_world && getenv("JX_VERIFY_DEATH_LOAD"))) && s==0x1f2a && o==0xa2f0 && n==0){
        unsigned prior_menu=h->menu_probe,prior_step=h->input_probe_step;
        if(h->adaptive_walkthrough && getenv("JX_VERIFY_DEATH_LOAD")){
            export_state(h,"main_decompiled/build/native_adaptive_death_before_load.bin");
            printf("native adaptive death load begin: name=%s phase=%u slot=%u battle=%u\n",h->adaptive_name,h->walkthrough_phase,h->adaptive_resume_slot,jx_word(&h->c,0x2934));fflush(stdout);
            h->menu_probe=5;h->input_probe_step=0;h->adaptive_skill_row=h->adaptive_resume_slot;h->adaptive_skill_column=0;
            key_scan(h,0x9c);
            uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            h->menu_probe=prior_menu;h->input_probe_step=prior_step;key_scan(h,0x9c);
            if((uint16_t)result!=0)fail(h,"adaptive death load did not select original save");
            export_state(h,"main_decompiled/build/native_adaptive_death_after_load.bin");
            printf("native adaptive death load complete: name=%s phase=%u slot=%u result=0 map=%u\n",h->adaptive_name,h->walkthrough_phase,h->adaptive_resume_slot,jx_word(&h->c,0x9334));fflush(stdout);
            if(getenv("JX_VERIFY_DEATH_LOAD_CONTINUE")){
                h->adaptive_death_loaded=1;
                return result;
            }
            longjmp(h->stop,3);
        }
        h->menu_probe=1;h->input_probe_step=0;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->menu_probe=prior_menu;h->input_probe_step=prior_step;return result;
    }
    if(h->scripted_battles && s==0x1f2a && o==0x4934 && n==3){
        unsigned prior=h->battle_input;h->battle_input=1;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->battle_input=prior;return result;
    }
    if(h->portable_media && s==0x2f28 && o==0xa2e6 && n==0)return 0;
    if(h->sound_enabled){h->trace_segment[h->trace_position%32]=s;h->trace_offset[h->trace_position%32]=o;h->trace_position++;}
    /* All native FILE operations use the same stdio handle adapter. Route
     * getc here before the optional recovered CRT stream bridge, whose
     * DOS buffer pointers are not allocated by this host FILE adapter. */
    if(s==0x1000 && o==0x1ef6 && n==2)return dispatch(h,s,o,a,n);
    if(h->sound_enabled && s==0x1f2a && o==0x18f2 && n==4){
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        sound_submit(h,a[0],(JxFarPointer){a[1],a[2]});return result;
    }
    if(s==0x17d2 && o==0x000c){
        if(h->boot_probe && h->story_probe && !h->opening_depth)export_state(h,"main_decompiled/build/native_opening_initial.bin");
        h->opening_depth++;
        uint32_t result=h->input_fallback(h->input_fallback_opaque,s,o,a,n);
        h->opening_depth--;
        if(h->boot_probe && h->story_probe && !h->opening_depth)export_state(h,"main_decompiled/build/native_opening_final.bin");
        return result;
    }
    if(h->adaptive_walkthrough && h->adaptive_name[0] && s==0x2f28 && (o==0x3b8e || o==0x7ec8)){
        printf("native adaptive cinematic: name=%s phase=%u service=%04X:%04X arg0=%u\n",h->adaptive_name,h->walkthrough_phase,s,o,n?a[0]:0);
        fflush(stdout);
    }
    if(h->ending_probe && s==0x1f2a && o==0xb196 && n==0)export_state(h,"main_decompiled/build/native_ending_shutdown_entry.bin");
    if(h->adaptive_walkthrough && s==0x1f2a && o==0xb196 && n==0){
        export_state(h,"main_decompiled/build/native_adaptive_ending_shutdown_entry.bin");
        printf("native adaptive ending shutdown entry: name=%s phase=%u credits_frames=%u credits_position=%u map=%u\n",h->adaptive_name,h->walkthrough_phase,h->credits_frames,h->credits_position,jx_word(&h->c,0x9334));
        fflush(stdout);
    }
    if(s==0x2f28 && o==0x7d4e && n==1){
        if(h->ending_probe && (h->credits_frames<2 || h->credits_frames==204 || h->credits_frames==205) && getenv("JX_TRACE_ENDING"))
            printf("ending credit frame=%u polls=%u\n",h->credits_frames,h->story_poll_steps);
        h->credits_position=a[0];h->credits_frames++;
        if(h->adaptive_walkthrough && (h->credits_frames==1 || h->credits_frames%256==0)) {
            printf("native adaptive credits progress: frames=%u position=%u polls=%u\n",h->credits_frames,h->credits_position,h->story_poll_steps);
            fflush(stdout);
        }
    }
    if(s==0x1f2a && o==0xc1f2 && n==2) {
        if(h->window.handle && !jx_native_window_pump(&h->window))window_shutdown(h);
        if(h->battle_input && !h->menu_probe){
            if(++h->story_poll_steps>200000)fail(h,"battle test input scenario exhausted");
            /* Credits consume one event per frame. Multiple active original
             * timers must not flood the queue at host polling speed. */
            if((!h->cutscene_fixture && !(h->adaptive_walkthrough && h->credits_frames)) || jx_word(&h->c,0x0864)==jx_word(&h->c,0x0866))timer_tick(h);
            /* In actual adaptive play, let the original credit scroll reach
             * its own final row before supplying the ordinary exit key. */
            int adaptive_full_credits=h->adaptive_walkthrough && h->credits_frames>0 && h->credits_position<3399;
            if(h->story_poll_steps%16==0 && !adaptive_full_credits && (!h->cutscene_fixture || (h->cutscene_fixture==1 && (h->credits_frames==0 || h->credits_frames>=h->credits_limit)))){if(h->window_probe)jx_native_window_post_scan(&h->window,0x1c);else key_scan(h,0x1c);}
            if(h->story_poll_steps%16==1 && h->story_poll_steps>1){if(h->window_probe)jx_native_window_post_scan(&h->window,0x9c);else key_scan(h,0x9c);}
        }
        if(h->menu_probe) {
            static const uint8_t direct[]={0x1c};
            static const uint8_t down[]={0x50,0xd0,0x1c};
            static const uint8_t cancel[]={1,0x81};
            static const uint8_t up[]={0x48,0xc8,0x1c};
            const uint8_t *scans=h->menu_probe==4?up:h->menu_probe==3?cancel:h->menu_probe==2?down:direct;
            unsigned count=h->menu_probe==4?sizeof up:h->menu_probe==3?sizeof cancel:h->menu_probe==2?sizeof down:sizeof direct;
            if(h->menu_probe==5)count=2*h->adaptive_skill_row+2*h->adaptive_skill_column+1;
            if(a[1]==0x9600)return h->input_fallback(h->input_fallback_opaque,s,o,a,n);
            if(h->input_probe_step>=count){
                if(h->window_probe){
                    if(++h->input_probe_step>10000)fail(h,"window title fixture timed out");
                    if(!jx_native_window_post_scan(&h->window,(h->input_probe_step&1)?0x1c:0x9c) || !jx_native_window_pump(&h->window))fail(h,"window repeat keyboard failed");
                    return h->input_fallback(h->input_fallback_opaque,s,o,a,n);
                }
                fail(h,"menu probe exhausted its keyboard scenario");
            }
            unsigned step=h->input_probe_step++;
            uint8_t scan=h->menu_probe==5?(step==count-1?0x1c:step<2*h->adaptive_skill_row?((step&1)?0xd0:0x50):((step&1)?0xcd:0x4d)):scans[step];
            if(h->window_probe){if(!jx_native_window_post_scan(&h->window,scan) || !jx_native_window_pump(&h->window))fail(h,"window keyboard fixture failed");}
            else key_scan(h,scan);
        }
    }
    return h->input_fallback(h->input_fallback_opaque,s,o,a,n);
}
static uint32_t cd_packet_dword(Host *h,JxFarPointer p,unsigned off) {
    uint32_t v=0;for(unsigned i=0;i<4;i++)v|=(uint32_t)jx_far_byte(&h->c,p,(uint16_t)(off+i))<<(8*i);return v;
}
static void cd_msf(Host *h,JxFarPointer p,unsigned off,uint32_t frames,int reversed) {
    uint8_t m=(uint8_t)(frames/4500),s=(uint8_t)(frames/75%60),f=(uint8_t)(frames%75);
    jx_far_set_byte(&h->c,p,(uint16_t)off,reversed?f:m);
    jx_far_set_byte(&h->c,p,(uint16_t)(off+1),s);
    jx_far_set_byte(&h->c,p,(uint16_t)(off+2),reversed?m:f);
    jx_far_set_byte(&h->c,p,(uint16_t)(off+3),0);
}
static void cd_query(void *p,JxCdInstallation *out) {
    Host *h=p;*out=(JxCdInstallation){h->cd.image?1:0,3};
}
static uint8_t cd_list(void *p,JxFarPointer out) {
    Host *h=p;for(unsigned i=0;i<5;i++)jx_far_set_byte(&h->c,out,(uint16_t)i,0);return h->cd.image?0:1;
}
static uint16_t cd_request(void *p,uint16_t drive,JxFarPointer packet) {
    Host *h=p;(void)drive;h->cd_requests++;
    uint16_t status=0x100;uint8_t command=jx_far_byte(&h->c,packet,2);
    if(!h->cd.image)status=0x8102;
    else if(command==3){
        uint32_t pointer=cd_packet_dword(h,packet,14);
        JxFarPointer out={(uint16_t)pointer,(uint16_t)(pointer>>16)};
        unsigned code=jx_far_byte(&h->c,out,0);uint32_t position=jx_native_cd_position(&h->cd);
        if(code==10){
            jx_far_set_byte(&h->c,out,1,(uint8_t)h->cd.first);jx_far_set_byte(&h->c,out,2,(uint8_t)h->cd.last);
            cd_msf(h,out,3,h->cd.sectors+150,1);
        }else if(code==11){
            unsigned track=jx_far_byte(&h->c,out,1);
            if(track<h->cd.first || track>h->cd.last)status=0x8103;
            else{cd_msf(h,out,2,h->cd.tracks[track]+150,1);jx_far_set_byte(&h->c,out,6,0);}
        }else if(code==12){
            unsigned track=h->cd.first;while(track<h->cd.last && h->cd.tracks[track+1]<=position)track++;
            jx_far_set_byte(&h->c,out,1,0);jx_far_set_byte(&h->c,out,2,(uint8_t)((track/10)*16+track%10));
            jx_far_set_byte(&h->c,out,3,1);cd_msf(h,out,4,position-h->cd.tracks[track],0);cd_msf(h,out,8,position+150,0);
        }else if(code==15){
            jx_far_set_byte(&h->c,out,1,(uint8_t)(h->cd.state==2));cd_msf(h,out,3,h->cd.start+150,0);
            cd_msf(h,out,7,h->cd.start+h->cd.length+150,0);
        }else status=0x8103;
    }else if(command==0x84){
        uint32_t start=cd_packet_dword(h,packet,14),count=cd_packet_dword(h,packet,18);
        if(jx_far_byte(&h->c,packet,13)==1){
            uint32_t frames=(start>>16&255)*4500+(start>>8&255)*75+(start&255);
            if(frames<150)status=0x8103;else start=frames-150;
        }
        if(status==0x100){
            if(!jx_native_cd_play(&h->cd,start,count,h->window.handle!=NULL))status=0x8103;
            else {
                unsigned narration=(uint8_t)(jx_ds(&h->c)[0x932c]+22);
                if(h->opening_depth && !h->story_probe && h->window.handle && count &&
                    narration<100 && start==h->cd.tracks[narration])
                    jx_native_window_sync_ticks(&h->window);
            }
        }
    }else if(command==0x85){if(!jx_native_cd_stop(&h->cd))status=0x8103;
    }else if(command==0x88){if(!jx_native_cd_resume(&h->cd))status=0x8103;
    }else status=0x8103;
    if(!(status&0x8000) && h->cd.state==1)status|=0x200;
    put_far_word(h,(uint16_t)(packet.offset+3),packet.segment,status);return 0;
}
/* Closing the host window cannot wait for an exit picture's keyboard input.
 * Run the recovered device cleanup entries, then the original exit service. */
static void window_shutdown(Host *h) {
    jx_shutdown_timer_callbacks(&h->c);
    jx_shutdown_keyboard_vector(&h->c);
    jx_shutdown_sound_storage(&h->c);
    if(jx_word(&h->c,0x31b6)){
        JxCleanupDevice device={h,sound_gate,cleanup_bios};
        jx_sound_restore_irq(&h->c,(JxFarPointer){0x31b8,h->c.data_segment},&device);
    }
    if(jx_word(&h->c,0x31b4))(void)JX_CALL(&h->c,0x1832,0x22ce);
    jx_shutdown_text_mode(&h->c);
    (void)JX_CALL(&h->c,0x1000,0x01eb,0);
}
static int input_poll(void *p,JxInputEvent *event) {
    Host *h=p;
    if(h->ui_events)return ui_test_input(h,event);
    if(h->story_probe || h->saved_walkthrough) {
        if(++h->story_poll_steps>200000){fprintf(stderr,"input exhausted: creditframes=%u pos=%u mode=%u enabled=%u head=%u tail=%u\n",h->credits_frames,h->credits_position,jx_word(&h->c,0x0e5a),jx_word(&h->c,0x0868),jx_word(&h->c,0x0864),jx_word(&h->c,0x0866));fail(h,"opening test input scenario exhausted");}
        if((!h->cutscene_fixture && !(h->adaptive_walkthrough && h->credits_frames)) || jx_word(&h->c,0x0864)==jx_word(&h->c,0x0866))timer_tick(h);
        int adaptive_full_credits=h->adaptive_walkthrough && h->credits_frames>0 && h->credits_position<3399;
        if(h->story_poll_steps%16==0 && !adaptive_full_credits && (!h->cutscene_fixture || (h->cutscene_fixture==1 && (h->credits_frames==0 || h->credits_frames>=h->credits_limit)))){if(h->window_probe)jx_native_window_post_scan(&h->window,0x1c);else key_scan(h,0x1c);}
        if(h->story_poll_steps%16==1 && h->story_poll_steps>1){if(h->window_probe)jx_native_window_post_scan(&h->window,0x9c);else key_scan(h,0x9c);}
        if(h->window_probe && !jx_native_window_pump(&h->window))fail(h,"window fixture closed during opening");
        int polled=jx_input_poll_queue(&h->c,event);
        return polled;
    }
    if(!h->window.handle)fail(h,"this operation requires interactive input (--play)");
    if(!jx_native_window_pump(&h->window))window_shutdown(h);
    return jx_input_poll_queue(&h->c,event);
}
static void export_ppm(Host *h,const char *path) {
    FILE *f=fopen(path,"wb");if(!f)fail(h,"cannot create preview");
    fprintf(f,"P6\n320 192\n255\n");unsigned page=jx_word(&h->c,0x3e54)%2;
    for(unsigned y=0;y<192;y++)for(unsigned x=0;x<320;x++) {
        unsigned index=h->video->planes[x%4][page*0x4000+y*80+x/4];
        for(unsigned rgb=0;rgb<3;rgb++)fputc(h->palette[index*3+rgb]*4,f);
    }
    if(fclose(f))fail(h,"preview write failed");
}
static void wav_word(uint8_t *p,uint16_t value){p[0]=(uint8_t)value;p[1]=(uint8_t)(value>>8);}
static void wav_long(uint8_t *p,uint32_t value){wav_word(p,(uint16_t)value);wav_word(p+2,(uint16_t)(value>>16));}
static void sound_submit(Host *h,uint16_t size,JxFarPointer descriptor) {
    uint16_t format=jx_far_word(&h->c,descriptor.segment,(uint16_t)(descriptor.offset+2));
    uint16_t bits=jx_far_byte(&h->c,descriptor,0),channels=jx_far_byte(&h->c,descriptor,1);
    uint32_t rate=jx_far_word(&h->c,descriptor.segment,(uint16_t)(descriptor.offset+6))|
        (uint32_t)jx_far_word(&h->c,descriptor.segment,(uint16_t)(descriptor.offset+8))<<16;
    if(format!=0 && format!=4)fail(h,"native PCM device does not accept compressed DSP formats");
    uint32_t at=(uint32_t)jx_word(&h->c,0x31cc)*16+jx_word(&h->c,0x31ca);
    if(!rate || !size || (bits!=8 && bits!=16) || (channels!=1 && channels!=2) || at+size>sizeof h->memory)
        fail(h,"invalid native PCM transfer");
    if(h->audio_probe){
        (void)_mkdir("main_decompiled/build/native_audio");
        char path[128];snprintf(path,sizeof path,"main_decompiled/build/native_audio/%04u.wav",h->audio_exports++);
        FILE *f=fopen(path,"wb");if(!f)fail(h,"cannot export native PCM transfer");
        uint8_t header[44]={0};memcpy(header,"RIFF",4);wav_long(header+4,36u+size);memcpy(header+8,"WAVEfmt ",8);
        wav_long(header+16,16);wav_word(header+20,1);wav_word(header+22,channels);wav_long(header+24,rate);
        wav_long(header+28,rate*channels*bits/8);wav_word(header+32,(uint16_t)(channels*bits/8));wav_word(header+34,bits);
        memcpy(header+36,"data",4);wav_long(header+40,size);
        if(fwrite(header,1,sizeof header,f)!=sizeof header || fwrite(h->memory+at,1,size,f)!=size || fclose(f))
            fail(h,"native PCM export failed");
    }else if(h->window.handle && !jx_native_audio_play(&h->audio,h->memory+at,size,rate,channels,bits))
        fail(h,"native audio output failed");
}
static void export_state(Host *h,const char *path) {
    for(unsigned i=0;i<4;i++)if(h->mapping[i]>=0)
        memcpy(h->banks[h->mapping[i]],h->memory+(uint32_t)h->ems_frame*16+i*16384,16384);
    FILE *f=fopen(path,"wb");if(!f)fail(h,"cannot create state dump");
    if(fwrite(h->memory,1,sizeof h->memory,f)!=sizeof h->memory ||
       fwrite(h->banks,1,sizeof h->banks,f)!=sizeof h->banks ||
       fwrite(h->video->planes,1,sizeof h->video->planes,f)!=sizeof h->video->planes ||
       fwrite(h->video->latches,1,4,f)!=4 || fwrite(h->palette,1,768,f)!=768)
        fail(h,"state dump write failed");
    if(fclose(f))fail(h,"state dump close failed");
    char metadata[1024];int length=snprintf(metadata,sizeof metadata,"%s.meta.json",path);
    if(length<0 || (size_t)length>=sizeof metadata)fail(h,"state path too long");
    f=fopen(metadata,"wb");if(!f)fail(h,"cannot create state metadata");
    fprintf(f,"{\"mapping\":[%d,%d,%d,%d],\"mask\":%u,\"read_map\":%u,\"mode\":%u,\"start\":%u,\"cd_requests\":%u,\"input_polls\":%u,\"cd_state\":%u,\"cd_start\":%u,\"cd_length\":%u,\"cd_elapsed\":%u,\"probe_clock_milliseconds\":%u,\"heap_blocks\":[",
        h->mapping[0],h->mapping[1],h->mapping[2],h->mapping[3],h->video->map_mask,h->video->read_map,
        h->video->mode,h->video->start_high,h->cd_requests,h->story_poll_steps,
        h->cd.state,h->cd.start,h->cd.length,h->cd.elapsed,h->probe_clock_milliseconds);
    unsigned emitted=0;
    for(unsigned i=0;i<0x4000;i++)if(h->heap_size[i])
        fprintf(f,"%s[%u,%u]",emitted++?",":"",0x5000+i,h->heap_size[i]);
    fprintf(f,"]}\n");
    if(fclose(f))fail(h,"state metadata close failed");
}
/* Original signed page remainder can address conventional RAM (9C000).
 * Device callbacks route physical addresses; RAM transfers do not touch latches. */
static int video_address_space_probe(Host *h,unsigned sample) {
    static const uint16_t pages[][2]={{0,0xffff},{0xffff,0},{0xffff,0xffff},{0,1},{1,0},{1,0xffff},{0xffff,1},{1,1}};
    if(sample>=sizeof pages/sizeof pages[0])return 1;
    FILE *f=fopen("save_tool/memory.bin","rb");
    if(!f || fread(h->memory,1,sizeof h->memory,f)!=sizeof h->memory)fail(h,"video probe missing baseline");
    fclose(f);
    for(unsigned at=0x9c000;at<0xa0000;at++)h->memory[at]=(uint8_t)(at*17u+sample*29u);
    h->c=(JxEventContext){h->memory,0x39e3,dispatch,h,host_save_browser,h};
    h->video=&h->startup.planar;jx_video_model_init(h->video);
    h->device=jx_video_model_bus(h->video);h->raw=(JxVideoBus){h,video_out,video_in,video_read,video_write};
    h->video->map_mask=15;h->video->read_map=(uint8_t)(sample%4);h->video->mode=0x40;
    for(unsigned plane=0;plane<4;plane++) {
        for(unsigned at=0;at<65536;at++)h->video->planes[plane][at]=(uint8_t)(at*7u+plane*53u+sample*11u);
        h->video->latches[plane]=(uint8_t)(plane*61u+sample);
    }
    jx_video_copy_page(&h->raw,pages[sample][0],pages[sample][1]);
    char path[128];snprintf(path,sizeof path,"main_decompiled/build/native_video_address_space_%u.bin",sample);
    f=fopen(path,"wb");if(!f)fail(h,"video probe output failed");
    if(fwrite(h->memory,1,sizeof h->memory,f)!=sizeof h->memory || fwrite(h->video->planes,1,sizeof h->video->planes,f)!=sizeof h->video->planes || fwrite(h->video->latches,1,4,f)!=4 || fputc(h->video->mode,f)==EOF || fclose(f))fail(h,"video probe write failed");
    printf("native video address space probe PASS: sample=%u source=%u destination=%u\n",sample,pages[sample][1],pages[sample][0]);
    return 0;
}
static void item_description_regression(Host *h) {
    uint8_t inventory[256];memcpy(inventory,jx_ds(&h->c)+0x878a,sizeof inventory);
    uint16_t battle=jx_word(&h->c,0xacea);memset(jx_ds(&h->c)+0x878a,0,sizeof inventory);
    static const uint16_t items[]={42,57,40,41,59,15};
    for(unsigned i=0;i<6;i++){jx_set_word(&h->c,(uint16_t)(0x878a+4*i),items[i]);jx_set_word(&h->c,(uint16_t)(0x878c+4*i),5);}
    static const uint16_t input[][2]={{1,0x4d},{1,0x50},{1,0x4b},{1,0x50},{1,0x4d},{1,1}};
    jx_set_word(&h->c,0xacea,1);h->ui_events=input;h->ui_count=6;h->ui_index=0;h->ui_name="item_effects";
    if(jx_item_menu(&h->c,(JxFarPointer){0,0x9c00})!=0xffff || h->ui_index!=6)fail(h,"item description navigation mismatch");
    for(unsigned i=0;i<6;i++)if(jx_word(&h->c,(uint16_t)(0x878c+4*i))!=5)fail(h,"description consumed inventory");
    h->ui_events=NULL;memcpy(jx_ds(&h->c)+0x878a,inventory,sizeof inventory);jx_set_word(&h->c,0xacea,battle);
    printf("native item description PASS: real medicine/maxima/stat/projectile/quest data, both columns, selection redraw and unchanged quantities\n");
}
static void menu_regression(Host *h) {
    static const uint16_t system_events[][2]={{1,1},{1,0x50},{1,0x50},{1,0x50},{0,0x1c},{0,0x1c},{1,0x50},{0,0x39},{1,0x50},{1,0x50},{1,0x50},{0,0x1c},{0,0x39},{1,1},{1,1}};
    uint16_t sound=jx_word(&h->c,0x050e),music=jx_word(&h->c,0x050c);
    h->ui_events=system_events;h->ui_count=sizeof system_events/sizeof system_events[0];h->ui_index=0;h->ui_name="system";
    if(jx_game_loop_step(&h->c,(JxFarPointer){0,0x9600})!=JX_GAME_LOOP_PROCESSED || h->ui_index!=h->ui_count)fail(h,"menu hierarchy return mismatch");
    if(jx_word(&h->c,0x050e)!=(uint16_t)(1-sound))fail(h,"sound toggle mismatch");
    if(jx_word(&h->c,0x050c)!=(jx_word(&h->c,0x31b4)?(uint16_t)(1-music):music))fail(h,"music toggle mismatch");
    static const uint16_t screens[][2]={{1,1},{0,0x39},{0,0x1c},{1,0x50},{0,0x1c},{1,1},{1,0x50},{0,0x39},{1,1},{1,1}};
    h->ui_events=screens;h->ui_count=sizeof screens/sizeof screens[0];h->ui_index=0;h->ui_name="screens";
    if(jx_game_loop_step(&h->c,(JxFarPointer){0,0x9600})!=JX_GAME_LOOP_PROCESSED || h->ui_index!=h->ui_count)fail(h,"status skill item return mismatch");
    h->window.video=h->video;h->window.palette=h->palette;h->window.mode=&h->legacy_mode;
    unsigned start=(unsigned)h->video->start_high<<8;
    for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++) {
        unsigned color=h->video->planes[x&3][(start+y*80+x/4)&65535];const uint8_t *rgb=h->palette+color*3;
        h->window.pixels[y*320+x]=((uint32_t)rgb[0]*4<<16)|((uint32_t)rgb[1]*4<<8)|(rgb[2]*4);
    }
    (void)jx_native_game_save_browser(&h->window,&h->c,2);
    static const uint16_t browser_events[][2]={{1,0x4b},{1,0x48},{1,1}};
    h->ui_events=browser_events;h->ui_count=3;h->ui_index=0;h->ui_name="browser";
    if(jx_native_game_save_browser(&h->window,&h->c,0)!=-1 || h->ui_index!=3)fail(h,"browser paging cancel mismatch");
    h->ui_events=NULL;printf("native menu system PASS: four main options, system toggles, safe exit default, status/skill/item returns, both confirmation keys, browser page 10 and cancellation\n");
}
int main(int argc,char **argv) {
    int resume_walk=argc>2 && strcmp(argv[2],"--adaptive-load-walkthrough-probe")==0;
    unsigned resume_phase=resume_walk && argc>3?(unsigned)strtoul(argv[3],NULL,0):0;
    unsigned resume_slot=resume_walk && argc>4?(unsigned)strtoul(argv[4],NULL,0):0;
    if(resume_walk && (argc!=5 || resume_phase<92 || resume_phase>10000 || resume_slot>4)){fprintf(stderr,"invalid adaptive load phase/slot\n");return 2;}
    setvbuf(stdout,NULL,_IONBF,0);
    Host *h=&host;unsigned place=argc>1?(unsigned)strtoul(argv[1],NULL,10):2;
    if(argc>2 && strcmp(argv[2],"--video-address-space-probe")==0)return video_address_space_probe(h,argc>3?(unsigned)strtoul(argv[3],NULL,10):0);
    int entry_window_probe=argc>2 && strcmp(argv[2],"--original-entry-window-probe")==0;
    int window_probe=entry_window_probe || (argc>2 && strcmp(argv[2],"--game-window-probe")==0);
    h->window_probe=(unsigned)window_probe;
    int sound_shutdown_probe=argc>2 && strcmp(argv[2],"--sound-shutdown-probe")==0;
    int sound_probe=argc>2 && strcmp(argv[2],"--sound-probe")==0;
    h->brothel_walkthrough=(unsigned)(argc>2 && strcmp(argv[2],"--brothel-walkthrough-probe")==0);
    h->saved_walkthrough=(unsigned)resume_walk;
    h->adaptive_walkthrough=(unsigned)(resume_walk || (argc>2 && strcmp(argv[2],"--adaptive-walkthrough-probe")==0));
    const char *supply_stage=getenv("JX_TEST_MAX_SUPPLIES_STAGE");
    if(supply_stage && *supply_stage){
        char *end=NULL;unsigned long stage=strtoul(supply_stage,&end,10);
        if(!h->adaptive_walkthrough || !stage || stage>99999 || !end || *end)fail(h,"invalid maximum supply test stage");
        h->test_supply_stage=(unsigned)stage;
    }
    h->wuyi_cave_key_second_walkthrough=(unsigned)(h->adaptive_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-cave-key-second-walkthrough-probe")==0));
    h->wuyi_cave_key_first_walkthrough=(unsigned)(h->wuyi_cave_key_second_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-cave-key-first-walkthrough-probe")==0));
    h->wuyi_inner_cave_entry_walkthrough=(unsigned)(h->wuyi_cave_key_first_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-inner-cave-entry-walkthrough-probe")==0));
    h->wuyi_rear_cave_entry_walkthrough=(unsigned)(h->wuyi_inner_cave_entry_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-rear-cave-entry-walkthrough-probe")==0));
    h->liu_wounded_departure_walkthrough=(unsigned)(h->wuyi_rear_cave_entry_walkthrough || (argc>2 && strcmp(argv[2],"--liu-wounded-departure-walkthrough-probe")==0));
    h->liu_wounded_clue_walkthrough=(unsigned)(h->liu_wounded_departure_walkthrough || (argc>2 && strcmp(argv[2],"--liu-wounded-clue-walkthrough-probe")==0));
    h->wuyi_second_liu_house_walkthrough=(unsigned)(h->liu_wounded_clue_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-second-liu-house-walkthrough-probe")==0));
    h->wuyi_killer_fifth_walkthrough=(unsigned)(h->wuyi_second_liu_house_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-killer-fifth-walkthrough-probe")==0));
    h->wuyi_killer_fourth_walkthrough=(unsigned)(h->wuyi_killer_fifth_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-killer-fourth-walkthrough-probe")==0));
    h->wuyi_killer_third_walkthrough=(unsigned)(h->wuyi_killer_fourth_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-killer-third-walkthrough-probe")==0));
    h->wuyi_killer_second_walkthrough=(unsigned)(h->wuyi_killer_third_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-killer-second-walkthrough-probe")==0));
    h->wuyi_killer_first_walkthrough=(unsigned)(h->wuyi_killer_second_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-killer-first-walkthrough-probe")==0));
    h->wuyi_second_disciple_walkthrough=(unsigned)(h->wuyi_killer_first_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-second-disciple-walkthrough-probe")==0));
    h->wuyi_second_entrance_walkthrough=(unsigned)(h->wuyi_second_disciple_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-second-entrance-walkthrough-probe")==0));
    h->linan_wuyi_departure_walkthrough=(unsigned)(h->wuyi_second_entrance_walkthrough || (argc>2 && strcmp(argv[2],"--linan-wuyi-departure-walkthrough-probe")==0));
    h->city_wall_departure_walkthrough=(unsigned)(h->linan_wuyi_departure_walkthrough || (argc>2 && strcmp(argv[2],"--city-wall-departure-walkthrough-probe")==0));
    h->city_wall_reunion_walkthrough=(unsigned)(h->city_wall_departure_walkthrough || (argc>2 && strcmp(argv[2],"--city-wall-reunion-walkthrough-probe")==0));
    h->city_wall_return_walkthrough=(unsigned)(h->city_wall_reunion_walkthrough || (argc>2 && strcmp(argv[2],"--city-wall-return-walkthrough-probe")==0));
    h->inn_city_return_walkthrough=(unsigned)(h->city_wall_return_walkthrough || (argc>2 && strcmp(argv[2],"--inn-city-return-walkthrough-probe")==0));
    h->inn_owner_release_walkthrough=(unsigned)(h->inn_city_return_walkthrough || (argc>2 && strcmp(argv[2],"--inn-owner-release-walkthrough-probe")==0));
    h->inn_downstairs_walkthrough=(unsigned)(h->inn_owner_release_walkthrough || (argc>2 && strcmp(argv[2],"--inn-downstairs-walkthrough-probe")==0));
    h->city_wall_zhang_walkthrough=(unsigned)(h->inn_downstairs_walkthrough || (argc>2 && strcmp(argv[2],"--city-wall-zhang-walkthrough-probe")==0));
    h->city_wall_entry_walkthrough=(unsigned)(h->city_wall_zhang_walkthrough || (argc>2 && strcmp(argv[2],"--city-wall-entry-walkthrough-probe")==0));
    h->he_mei_departure_walkthrough=(unsigned)(h->city_wall_entry_walkthrough || (argc>2 && strcmp(argv[2],"--he-mei-departure-walkthrough-probe")==0));
    h->he_mei_gift_walkthrough=(unsigned)(h->he_mei_departure_walkthrough || (argc>2 && strcmp(argv[2],"--he-mei-gift-walkthrough-probe")==0));
    h->he_mei_proposal_walkthrough=(unsigned)(h->he_mei_gift_walkthrough || (argc>2 && strcmp(argv[2],"--he-mei-proposal-walkthrough-probe")==0));
    h->he_mei_house_walkthrough=(unsigned)(h->he_mei_proposal_walkthrough || (argc>2 && strcmp(argv[2],"--he-mei-house-walkthrough-probe")==0));
    h->rescued_city_return_walkthrough=(unsigned)(h->he_mei_house_walkthrough || (argc>2 && strcmp(argv[2],"--rescued-city-return-walkthrough-probe")==0));
    h->village_reunion_walkthrough=(unsigned)(h->rescued_city_return_walkthrough || (argc>2 && strcmp(argv[2],"--village-reunion-walkthrough-probe")==0));
    h->island_sailing_return_walkthrough=(unsigned)(h->village_reunion_walkthrough || (argc>2 && strcmp(argv[2],"--island-sailing-return-walkthrough-probe")==0));
    h->island_hideout_return_walkthrough=(unsigned)(h->island_sailing_return_walkthrough || (argc>2 && strcmp(argv[2],"--island-hideout-return-walkthrough-probe")==0));
    h->island_cellar_return_walkthrough=(unsigned)(h->island_hideout_return_walkthrough || (argc>2 && strcmp(argv[2],"--island-cellar-return-walkthrough-probe")==0));
    h->island_he_mei_walkthrough=(unsigned)(h->island_cellar_return_walkthrough || (argc>2 && strcmp(argv[2],"--island-he-mei-walkthrough-probe")==0));
    h->island_cellar_walkthrough=(unsigned)(h->island_he_mei_walkthrough || (argc>2 && strcmp(argv[2],"--island-cellar-walkthrough-probe")==0));
    h->island_door_walkthrough=(unsigned)(h->island_cellar_walkthrough || (argc>2 && strcmp(argv[2],"--island-door-walkthrough-probe")==0));
    h->island_zhang_walkthrough=(unsigned)(h->island_door_walkthrough || (argc>2 && strcmp(argv[2],"--island-zhang-walkthrough-probe")==0));
    h->island_rescue_walkthrough=(unsigned)(h->island_zhang_walkthrough || (argc>2 && strcmp(argv[2],"--island-rescue-walkthrough-probe")==0));
    h->island_hideout_walkthrough=(unsigned)(h->island_rescue_walkthrough || (argc>2 && strcmp(argv[2],"--island-hideout-walkthrough-probe")==0));
    h->island_clearance_walkthrough=(unsigned)(h->island_hideout_walkthrough || (argc>2 && strcmp(argv[2],"--island-clearance-walkthrough-probe")==0));
    h->island_killer_jia_walkthrough=(unsigned)(h->island_clearance_walkthrough || (argc>2 && strcmp(argv[2],"--island-killer-jia-walkthrough-probe")==0));
    h->island_killer_yi_walkthrough=(unsigned)(h->island_killer_jia_walkthrough || (argc>2 && strcmp(argv[2],"--island-killer-yi-walkthrough-probe")==0));
    h->island_killer_bing_walkthrough=(unsigned)(h->island_killer_yi_walkthrough || (argc>2 && strcmp(argv[2],"--island-killer-bing-walkthrough-probe")==0));
    h->island_killer_wu_walkthrough=(unsigned)(h->island_killer_bing_walkthrough || (argc>2 && strcmp(argv[2],"--island-killer-wu-walkthrough-probe")==0));
    h->island_killer_walkthrough=(unsigned)(h->island_killer_wu_walkthrough || (argc>2 && strcmp(argv[2],"--island-killer-walkthrough-probe")==0));
    h->island_sailing_walkthrough=(unsigned)(h->island_killer_walkthrough || (argc>2 && strcmp(argv[2],"--island-sailing-walkthrough-probe")==0));
    h->sailing_pier_return_walkthrough=(unsigned)(h->island_sailing_walkthrough || (argc>2 && strcmp(argv[2],"--sailing-pier-return-walkthrough-probe")==0));
    h->sailing_wang_walkthrough=(unsigned)(h->sailing_pier_return_walkthrough || (argc>2 && strcmp(argv[2],"--sailing-wang-walkthrough-probe")==0));
    h->sailing_house_walkthrough=(unsigned)(h->sailing_wang_walkthrough || (argc>2 && strcmp(argv[2],"--sailing-house-walkthrough-probe")==0));
    h->sailing_village_walkthrough=(unsigned)(h->sailing_house_walkthrough || (argc>2 && strcmp(argv[2],"--sailing-village-walkthrough-probe")==0));
    h->jia_cup_walkthrough=(unsigned)(h->sailing_village_walkthrough || (argc>2 && strcmp(argv[2],"--jia-cup-walkthrough-probe")==0));
    h->brothel_city_return_walkthrough=(unsigned)(h->jia_cup_walkthrough || (argc>2 && strcmp(argv[2],"--brothel-city-return-walkthrough-probe")==0));
    h->brothel_downstairs_walkthrough=(unsigned)(h->brothel_city_return_walkthrough || (argc>2 && strcmp(argv[2],"--brothel-downstairs-walkthrough-probe")==0));
    h->nangong_cup_walkthrough=(unsigned)(h->brothel_downstairs_walkthrough || (argc>2 && strcmp(argv[2],"--nangong-cup-walkthrough-probe")==0));
    h->nangong_entry_walkthrough=(unsigned)(h->nangong_cup_walkthrough || (argc>2 && strcmp(argv[2],"--nangong-entry-walkthrough-probe")==0));
    h->jia_clue_walkthrough=(unsigned)(h->nangong_entry_walkthrough || (argc>2 && strcmp(argv[2],"--jia-clue-walkthrough-probe")==0));
    h->village_return_walkthrough=(unsigned)(h->jia_clue_walkthrough || (argc>2 && strcmp(argv[2],"--village-return-walkthrough-probe")==0));
    h->old_wang_return_walkthrough=(unsigned)(h->village_return_walkthrough || (argc>2 && strcmp(argv[2],"--old-wang-return-walkthrough-probe")==0));
    h->old_wang_clue_walkthrough=(unsigned)(h->old_wang_return_walkthrough || (argc>2 && strcmp(argv[2],"--old-wang-clue-walkthrough-probe")==0));
    h->old_wang_house_walkthrough=(unsigned)(h->old_wang_clue_walkthrough || (argc>2 && strcmp(argv[2],"--old-wang-house-walkthrough-probe")==0));
    h->fisherman_clue_walkthrough=(unsigned)(h->old_wang_house_walkthrough || (argc>2 && strcmp(argv[2],"--fisherman-clue-walkthrough-probe")==0));
    h->fishing_village_walkthrough=(unsigned)(h->fisherman_clue_walkthrough || (argc>2 && strcmp(argv[2],"--fishing-village-walkthrough-probe")==0));
    h->xiao_lei_walkthrough=(unsigned)(h->fishing_village_walkthrough || (argc>2 && strcmp(argv[2],"--xiao-lei-walkthrough-probe")==0));
    h->token_guard_walkthrough=(unsigned)(h->xiao_lei_walkthrough || (argc>2 && strcmp(argv[2],"--token-guard-walkthrough-probe")==0));
    h->casino_return_walkthrough=(unsigned)(h->token_guard_walkthrough || (argc>2 && strcmp(argv[2],"--casino-return-walkthrough-probe")==0));
    h->casino_token_walkthrough=(unsigned)(h->casino_return_walkthrough || (argc>2 && strcmp(argv[2],"--casino-token-walkthrough-probe")==0));
    h->casino_walkthrough=(unsigned)(h->casino_token_walkthrough || (argc>2 && strcmp(argv[2],"--casino-walkthrough-probe")==0));
    h->tavern_departure_walkthrough=(unsigned)(h->casino_walkthrough || (argc>2 && strcmp(argv[2],"--tavern-departure-walkthrough-probe")==0));
    h->tavern_downstairs_walkthrough=(unsigned)(h->tavern_departure_walkthrough || (argc>2 && strcmp(argv[2],"--tavern-downstairs-walkthrough-probe")==0));
    h->tavern_story_walkthrough=(unsigned)(h->tavern_downstairs_walkthrough || (argc>2 && strcmp(argv[2],"--tavern-story-walkthrough-probe")==0));
    h->tavern_upstairs_walkthrough=(unsigned)(h->tavern_story_walkthrough || (argc>2 && strcmp(argv[2],"--tavern-upstairs-walkthrough-probe")==0));
    h->tavern_clue_walkthrough=(unsigned)(h->tavern_upstairs_walkthrough || (argc>2 && strcmp(argv[2],"--tavern-clue-walkthrough-probe")==0));
    h->tavern_walkthrough=(unsigned)(h->tavern_clue_walkthrough || (argc>2 && strcmp(argv[2],"--tavern-walkthrough-probe")==0));
    h->city_information_walkthrough=(unsigned)(h->tavern_walkthrough || h->brothel_walkthrough || (argc>2 && strcmp(argv[2],"--city-information-walkthrough-probe")==0));
    h->linan_gate_walkthrough=(unsigned)(h->city_information_walkthrough || (argc>2 && strcmp(argv[2],"--linan-gate-walkthrough-probe")==0));
    h->linan_walkthrough=(unsigned)(h->linan_gate_walkthrough || (argc>2 && strcmp(argv[2],"--linan-walkthrough-probe")==0));
    h->wuyi_return_walkthrough=(unsigned)(h->linan_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-return-walkthrough-probe")==0));
    h->liu_return_walkthrough=(unsigned)(h->wuyi_return_walkthrough || (argc>2 && strcmp(argv[2],"--liu-return-walkthrough-probe")==0));
    h->zhang_lin_walkthrough=(unsigned)(h->liu_return_walkthrough || (argc>2 && strcmp(argv[2],"--zhang-lin-walkthrough-probe")==0));
    h->liu_house_walkthrough=(unsigned)(h->zhang_lin_walkthrough || (argc>2 && strcmp(argv[2],"--liu-house-walkthrough-probe")==0));
    h->wuyi_walkthrough=(unsigned)(h->liu_house_walkthrough || (argc>2 && strcmp(argv[2],"--wuyi-walkthrough-probe")==0));
    h->road_walkthrough=(unsigned)(h->wuyi_walkthrough || (argc>2 && strcmp(argv[2],"--road-walkthrough-probe")==0));
    h->east_walkthrough=(unsigned)(h->road_walkthrough || (argc>2 && strcmp(argv[2],"--east-walkthrough-probe")==0));
    h->blood_walkthrough=(unsigned)(h->east_walkthrough || (argc>2 && strcmp(argv[2],"--blood-walkthrough-probe")==0));
    h->return_walkthrough=(unsigned)(h->blood_walkthrough || (argc>2 && strcmp(argv[2],"--return-walkthrough-probe")==0));
    h->disciple_walkthrough=(unsigned)(h->return_walkthrough || (argc>2 && strcmp(argv[2],"--disciple-walkthrough-probe")==0));
    h->hall_walkthrough=(unsigned)(h->disciple_walkthrough || (argc>2 && strcmp(argv[2],"--hall-walkthrough-probe")==0));
    h->temple_walkthrough=(unsigned)(h->hall_walkthrough || (argc>2 && strcmp(argv[2],"--temple-walkthrough-probe")==0));
    h->walkthrough_probe=(unsigned)(h->temple_walkthrough || (argc>2 && strcmp(argv[2],"--walkthrough-probe")==0));
    h->original_entry_probe=(unsigned)(h->walkthrough_probe || entry_window_probe || (argc>2 && strcmp(argv[2],"--original-entry-probe")==0));
    if(resume_walk)h->original_entry_probe=0;
    int original_init_probe=h->original_entry_probe || (argc>2 && strcmp(argv[2],"--original-initialize-probe")==0);
    h->original_init_probe=(unsigned)original_init_probe;
    if(original_init_probe || resume_walk)h->cd.clock_milliseconds=fixed_cd_clock;
    h->sound_enabled=(unsigned)(original_init_probe || sound_probe || sound_shutdown_probe || window_probe || (argc>2 && strcmp(argv[2],"--game")==0));h->audio_probe=(unsigned)(sound_probe || sound_shutdown_probe);h->pic[0]=h->pic[1]=0xff;
    int use_item_probe=argc>2 && strcmp(argv[2],"--use-item-probe")==0;
    int pickup_probe=use_item_probe || (argc>2 && strcmp(argv[2],"--pickup-probe")==0);
    int board_window_probe=argc>2 && strcmp(argv[2],"--board-window-probe")==0;
    int board_probe=board_window_probe || (argc>2 && strcmp(argv[2],"--board-probe")==0);
    int dice_probe=argc>2 && strcmp(argv[2],"--dice-probe")==0;
    int ending_walk_probe=argc>2 && strcmp(argv[2],"--ending-walk-probe")==0;
    int ending_film_probe=argc>2 && strcmp(argv[2],"--ending-film-probe")==0;
    int ending_reunion_probe=argc>2 && strcmp(argv[2],"--ending-reunion-probe")==0;
    int ending_survivor_probe=argc>2 && strcmp(argv[2],"--ending-survivor-probe")==0;
    int final_battles_probe=argc>2 && strcmp(argv[2],"--final-battles-probe")==0;
    int final_death_probe=argc>2 && strcmp(argv[2],"--final-death-probe")==0;
    int ending_probe=ending_survivor_probe || ending_reunion_probe || ending_walk_probe || ending_film_probe || (argc>2 && strcmp(argv[2],"--ending-probe")==0);
    h->ending_probe=(unsigned)ending_probe;
    int ending_animation_probe=argc>2 && strcmp(argv[2],"--ending-animation-probe")==0;
    int credits_probe=argc>2 && strcmp(argv[2],"--credits-probe")==0;
    int cd_probe=argc>2 && strcmp(argv[2],"--cd-probe")==0;
    int disc_probe=argc>2 && strcmp(argv[2],"--disc-probe")==0;
    int menus_probe=argc>2 && strcmp(argv[2],"--menus-probe")==0;
    int board_quest_probe=argc>2 && strcmp(argv[2],"--board-quest-probe")==0;
    int cinematic_probe=argc>2 && strcmp(argv[2],"--cinematic-probe")==0;
    int fixed_animation_probe=argc>2 && strcmp(argv[2],"--fixed-animation-probe")==0;
    int world_contact_probe=argc>2 && (strcmp(argv[2],"--npc-world-contact-probe")==0 || strcmp(argv[2],"--trigger-world-contact-probe")==0);
    int walk_contact_probe=world_contact_probe || (argc>2 && (strcmp(argv[2],"--npc-walk-contact-probe")==0 || strcmp(argv[2],"--trigger-walk-contact-probe")==0));
    int contact_callback_probe=walk_contact_probe || (argc>2 && (strcmp(argv[2],"--npc-contact-probe")==0 || strcmp(argv[2],"--trigger-contact-probe")==0));
    int trigger_callback_probe=argc>2 && (strcmp(argv[2],"--trigger-callback-probe")==0 || strcmp(argv[2],"--trigger-contact-probe")==0 || strcmp(argv[2],"--trigger-walk-contact-probe")==0 || strcmp(argv[2],"--trigger-world-contact-probe")==0);
    int npc_callback_probe=contact_callback_probe || trigger_callback_probe || (argc>2 && strcmp(argv[2],"--npc-callback-probe")==0);
    int scene_probe=npc_callback_probe || fixed_animation_probe || final_death_probe || final_battles_probe || ending_survivor_probe || ending_reunion_probe || cinematic_probe || board_quest_probe || ending_walk_probe || ending_film_probe || pickup_probe || (argc>2 && strcmp(argv[2],"--scene-probe")==0);
    int interact_probe=argc>2 && strcmp(argv[2],"--interact-probe")==0;
    int story_continuation_probe=argc>2 && strcmp(argv[2],"--story-continuation-probe")==0;
    int battle_probe=argc>2 && strcmp(argv[2],"--battle-probe")==0;
    int shutdown_probe=sound_shutdown_probe || (argc>2 && strcmp(argv[2],"--shutdown-probe")==0);
    int item_description_probe=argc>2 && strcmp(argv[2],"--item-description-probe")==0;
    int menu_system_probe=argc>2 && strcmp(argv[2],"--menu-system-probe")==0;
    int main_menu_probe=argc>2 && strcmp(argv[2],"--main-menu-probe")==0;
    int save_probe=argc>2 && strcmp(argv[2],"--save-probe")==0;
    int load_probe=resume_walk || shutdown_probe || main_menu_probe || save_probe || (argc>2 && strcmp(argv[2],"--load-probe")==0);
    int game_probe=item_description_probe || menu_system_probe || original_init_probe || board_probe || dice_probe || ending_probe || ending_animation_probe || credits_probe || cd_probe || disc_probe || menus_probe || scene_probe || interact_probe || story_continuation_probe || window_probe || sound_probe || battle_probe || load_probe || (argc>2 && strcmp(argv[2],"--game-probe")==0);
    int game=game_probe || (argc>2 && strcmp(argv[2],"--game")==0);
    h->portable_media=(unsigned)(disc_probe || scene_probe || window_probe || (argc>2 && strcmp(argv[2],"--game")==0));
    for(int option=3;option<argc;option++)if(strcmp(argv[option],"--strict-cd")==0)h->portable_media=0;
    int play=window_probe || (game && !game_probe) || (argc>2 && strcmp(argv[2],"--play")==0);
    int boot=game || (argc>2 && strcmp(argv[2],"--boot")==0);
    h->boot_probe=(unsigned)(boot && (!game || game_probe));
    h->boot_seed=game_probe && argc>5?(unsigned)strtoul(argv[5],NULL,0):1;
    unsigned ticks=game_probe?64:argc>3?(unsigned)strtoul(argv[3],NULL,10):0;
    int scheduled=play || game_probe || (!boot && argc>3);
    if(place>=220){fprintf(stderr,"place must be 0..219\n");return 2;}
    int stopped=setjmp(h->stop);
    if(stopped) {
        jx_native_audio_close(&h->audio);
        jx_native_cd_close(&h->cd);
        if(!window_probe || stopped==3)jx_native_window_close(&h->window);
        if(play)printf("native window: paints=%u timer_messages=%u\n",h->window.paints,h->window.ticks);
        for(unsigned i=1;i<128;i++)if(h->files[i])fclose(h->files[i]);
        if(stopped==2){if(h->ending_probe){if(h->exit_status || h->credits_frames<h->credits_limit)return 1;printf("native ending dialogue/images/credits/shutdown PASS: polls=%u credits=%u\n",h->story_poll_steps,h->credits_frames);}
            if(h->adaptive_walkthrough && h->credits_frames)printf("native adaptive ending termination: name=%s phase=%u status=%u credits_frames=%u credits_position=%u\n",h->adaptive_name,h->walkthrough_phase,h->exit_status,h->credits_frames,h->credits_position);
            printf("native game terminated: status=%u\n",h->exit_status);return (int)h->exit_status;}
        if(stopped==3 && (resume_walk || (h->original_entry_probe && h->entry_ticks>=64)))return 0;
        return 1;
    }
    FILE *snapshot=fopen("save_tool/memory.bin","rb");
    if(!snapshot || fread(h->memory,1,sizeof h->memory,snapshot)!=sizeof h->memory)fail(h,"missing initial data snapshot");
    fclose(snapshot);h->ems_frame=boot?0xd000:0x7000;for(unsigned i=0;i<4;i++)h->mapping[i]=-1;
    h->c=(JxEventContext){h->memory,0x39e3,dispatch,h,host_save_browser,h};
    if(boot)jx_crt_clear_bss(&h->c);
    h->raw=(JxVideoBus){h,video_out,video_in,video_read,video_write};
    jx_video_startup_model_init(&h->startup);h->video=&h->startup.planar;
    h->device=jx_video_startup_model_bus(&h->startup);
    if(boot)legacy_video_initialize(h);
    jx_set_word(&h->c,0x2eb0,0);jx_set_word(&h->c,0x2eb2,0x6000);
    jx_set_word(&h->c,0x9334,(uint16_t)place);jx_set_word(&h->c,0x31b6,0);
    jx_set_word(&h->c,0xb684,0);jx_ds(&h->c)[0xb6a3]=0;
    jx_set_word(&h->c,0x3456,0);jx_set_word(&h->c,0x0ddc,0);jx_set_word(&h->c,0x08c8,0);jx_set_word(&h->c,0x3e54,0);
    double weight=0.3;memcpy(jx_ds(&h->c)+0x238c,&weight,sizeof weight);
    for(unsigned i=0;i<3;i++) {
        jx_set_word(&h->c,(uint16_t)(0x875a+2*i),(uint16_t)i);
        jx_set_word(&h->c,(uint16_t)(0x8760+12*i),(uint16_t)(15+i));
        jx_set_word(&h->c,(uint16_t)(0x8762+12*i),10);jx_set_word(&h->c,(uint16_t)(0x8764+12*i),0);
        jx_set_word(&h->c,(uint16_t)(0x8784+2*i),2);
    }
    JxVideoBridge video;JxEngineBridge scene,runtime,graphics,places,callbacks,inventory,overlay;
    JxResourceBridge resource;JxAnimationBridge animation;
    JxInputHardwareBridge input;JxInputWaitBridge waits;
    JxVideoStartupBridge startup;
    JxCdBridge cd;JxCutsceneBridge cutscene;JxDiceBridge dice;JxDiceGameBridge dice_game;
    JxFixedAnimationBridge fixed_animation;
    JxGameControlBridge game_control;
    JxChapterBridge chapter;JxNpcEraseBridge erase;
    JxMenuBridge menu;
    JxBattleBridge battle;
    JxSoundBridge sound;JxPortBridge ports;JxVocBridge voc;
    JxEngineBridge world;JxWorldCollisionBridge collision;
    JxAnimationServices services={h,dos_open,dos_allocate,dos_read,dos_seek,dos_close,dos_free};
    jx_video_bridge_install(&h->c,&video,&h->raw);
    jx_engine_bridge_install(&h->c,&scene);jx_runtime_bridge_install(&h->c,&runtime);
    jx_graphics_memory_bridge_install(&h->c,&graphics);
    jx_resource_bridge_install(&h->c,&resource,(JxFarPointer){0,0x9000},(JxFarPointer){0x100,0x9000},(JxFarPointer){0x200,0x9000});
    jx_animation_bridge_install(&h->c,&animation,&services);
    jx_place_bridge_install(&h->c,&places);jx_callback_bridge_install(&h->c,&callbacks);jx_npc_erase_bridge_install(&h->c,&erase);
    jx_inventory_bridge_install(&h->c,&inventory);jx_overlay_event_bridge_install(&h->c,&overlay);
    h->ports=(JxPortDevice){h,port_read,port_write};
    jx_port_bridge_install(&h->c,&ports,&h->ports);
    jx_sound_bridge_install(&h->c,&sound,(JxSoundDmaPorts){0,0,0},(JxSoundDspScratch){0,0});
    jx_voc_bridge_install(&h->c,&voc,(JxFarPointer){0,0xb300});
    JxInputHardwareDevice hardware={h,keyboard_bios,get_vector,set_vector,interrupt_disable};
    jx_input_hardware_bridge_install(&h->c,&input,&h->ports,&hardware,(JxFarPointer){0,0x9300});
    jx_input_wait_bridge_install(&h->c,&waits,input_poll,h);
    jx_world_update_bridge_install(&h->c,&world);
    jx_world_collision_bridge_install(&h->c,&collision,(JxFarPointer){0,0x9100});
    jx_video_startup_bridge_install(&h->c,&startup,&h->raw,native_video_bios,h);
    JxCdDevice cd_device={h,cd_request};
    jx_cd_bridge_install(&h->c,&cd,&cd_device,(JxFarPointer){0,0x9700},(JxFarPointer){0x200,0x9700});
    jx_chapter_bridge_install(&h->c,&chapter,(JxFarPointer){0,0x9800});
    jx_menu_bridge_install(&h->c,&menu,(JxFarPointer){0,0x9a00},(JxFarPointer){0,0x9d00});
    jx_battle_bridge_install(&h->c,&battle,(JxFarPointer){0,0xb000},
        JX_BATTLE_CONTROL|JX_BATTLE_MENUS|JX_BATTLE_IMAGES|JX_BATTLE_DRAW|JX_BATTLE_CACHE|JX_BATTLE_SETTLEMENT);
    jx_cutscene_bridge_install(&h->c,&cutscene,(JxFarPointer){0x300,0x9f00});
    jx_animation_fixed_bridge_install(&h->c,&fixed_animation);
    jx_game_control_bridge_install(&h->c,&game_control);
    jx_dice_bridge_install(&h->c,&dice,(JxFarPointer){0x400,0x9f00});
    jx_dice_game_bridge_install(&h->c,&dice_game,(JxFarPointer){0x500,0x9f00});
    h->input_fallback=h->c.call;h->input_fallback_opaque=h->c.opaque;
    h->c.call=native_input_dispatch;h->c.opaque=h;
    if(game && (!game_probe || window_probe || disc_probe || cd_probe || original_init_probe || resume_walk)){
        if(jx_native_cd_open(&h->cd,"../AudioCD.cue"))h->portable_media=0;
    }
    if((game && !game_probe) || entry_window_probe){
        h->window.video=h->video;h->window.palette=h->palette;h->window.opaque=h;h->window.legacy=&h->legacy_vga;h->window.mode=&h->legacy_mode;
        h->window.scan=key_scan;h->window.tick=timer_tick;
        if(!jx_native_window_open(&h->window))fail(h,"cannot open original entry display");
        if(!entry_window_probe){
            h->interactive_entry=1;h->interactive_limit=ticks;
            (void)JX_CALL0(&h->c,0x1f2a,0xb1fa);
            fail(h,"interactive original entry unexpectedly returned");
        }
    }
    if(original_init_probe){
        if(!h->cd.image)fail(h,"original initializer probe requires supplied audio CD image");
        export_state(h,"main_decompiled/build/native_original_initialize_initial.bin");
        h->story_probe=1;h->battle_input=1;h->scripted_battles=1;h->story_poll_steps=0;
        if(h->original_entry_probe){
            (void)JX_CALL0(&h->c,0x1f2a,0xb1fa);
            fail(h,"original main entry unexpectedly returned");
        }
        (void)JX_CALL0(&h->c,0x1f2a,0x6b38);
        h->story_probe=0;h->battle_input=0;h->original_init_probe=0;
        export_state(h,"main_decompiled/build/native_original_initialize_final.bin");
        printf("native original6B38 initialization PASS: place=%u polls=%u CD=%u sound=%u\n",jx_word(&h->c,0x9334),h->story_poll_steps,jx_word(&h->c,0x31b4),jx_word(&h->c,0x31b6));
        goto world_ready;
    }
    if(boot) {
        jx_configuration_load(&h->c);
        jx_set_word(&h->c,0x31b4,h->cd.image?1:0);
        /* Configure the emulated DSP before the original heap allocation.
         * The original CD image supplies the music device when available. */
        jx_set_word(&h->c,0x31b6,h->sound_enabled?1:0);jx_set_word(&h->c,0x050c,h->cd.image?1:0);
        if(h->sound_enabled){
            jx_set_word(&h->c,0x31b8,0x220);jx_set_word(&h->c,0x31ba,1);
            jx_set_word(&h->c,0x31bc,5);jx_set_word(&h->c,0x31c0,5);jx_set_word(&h->c,0x050e,1);
        }
        if(h->cd.image){
            JxCdDiscovery discovery={h,cd_query,cd_list};
            if(jx_cd_initialize_current(&h->c,&discovery,(JxFarPointer){0x200,0x9700}))fail(h,"CD discovery failed");
            jx_set_word(&h->c,0x31b4,1);
            if(jx_cd_current_disc(&h->c,&cd_device,(JxFarPointer){0x932c,h->c.data_segment}))fail(h,"CD disc query failed");
            if(JX_CALL(&h->c,0x2f28,0x866a))fail(h,"original CD track check failed");
            printf("native CD image: tracks=%u..%u sectors=%u original_check=PASS\n",h->cd.first,h->cd.last,h->cd.sectors);
        }
        if(jx_boot_memory_initialize(&h->c,bios_ticks,h))fail(h,"boot allocation stopped");
    }
    if(cd_probe){
        if(!h->cd.image)fail(h,"CD probe requires original image");
        uint8_t pcm[2352];
        for(unsigned track=h->cd.first;track<=h->cd.last;track++){
            if(JX_CALL(&h->c,0x1832,0x224c,track,0,1) || h->cd.start!=h->cd.tracks[track] || h->cd.length!=75 || h->cd.state!=1)fail(h,"CD track playback mismatch");
            if(!jx_native_cd_read(&h->cd,h->cd.tracks[track],1,pcm))fail(h,"CD PCM read failed");
            if(JX_CALL(&h->c,0x1832,0x22ce) || h->cd.state!=2)fail(h,"CD pause failed");
            if(JX_CALL(&h->c,0x1832,0x2180,0x300,0x9700) || jx_far_word(&h->c,0x9700,0x300)!=1)fail(h,"CD pause status mismatch");
            if(JX_CALL(&h->c,0x1832,0x2324) || h->cd.state!=1)fail(h,"CD resume failed");
        }
        if(!jx_cd_play(&h->c,&cd_device,0,h->cd.sectors,1,0))fail(h,"CD out-of-range request accepted");
        printf("native CD probe PASS: tracks=%u plays=%u requests=%u PCM=44100Hz/stereo/16bit\n",h->cd.last,h->cd.plays,h->cd_requests);
        jx_native_cd_close(&h->cd);return 0;
    }
    if(scheduled) {
        /* Enable music only when the original audio image is available. */
        jx_set_word(&h->c,0x050c,h->cd.image?1:0);
        /* Begin with the original installation functions, not guessed
         * direction states or an invented timer-event queue. */
        jx_set_word(&h->c,0x04f2,0);jx_set_word(&h->c,0x0e5a,0);
        jx_set_word(&h->c,0x0868,1);
        jx_keyboard_initialize(&h->c,keyboard_bios,h);jx_timer_initialize(&h->c);
    }
    if(jx_resources_initialize(&h->c,(JxFarPointer){0x400,0x9000}))fail(h,"resource initialization terminated");
    if(h->sound_enabled)jx_sound_initialize(&h->c);
    if(boot) {
        jx_graphics_initialize(&h->c);
        if(game && (!game_probe || window_probe)) {
            h->window.video=h->video;h->window.palette=h->palette;h->window.opaque=h;h->window.legacy=&h->legacy_vga;h->window.mode=&h->legacy_mode;
            h->window.scan=key_scan;h->window.tick=timer_tick;
            if(!jx_native_window_open(&h->window))fail(h,"cannot open native game display");
        }
        jx_game_database_load(&h->c);
        if(game) {
            jx_image_tail_load(&h->c,(JxFarPointer){0x04ca,h->c.data_segment});
            h->menu_probe=load_probe?2:(unsigned)game_probe;
            uint16_t selected=jx_two_choice_menu(&h->c,200,50,(JxFarPointer){0x04db,h->c.data_segment},(JxFarPointer){0x04d4,h->c.data_segment},(JxFarPointer){0,0x9a00});
            h->menu_probe=0;if(game_probe)key_scan(h,0x9c);
            if(selected!=0) {
                jx_font_set_colors(&h->c,80,0,77);jx_font_set_mode(&h->c,176);
                (void)JX_CALL(&h->c,0x1832,0x1884,0,0,320,200,0);
                jx_palette_write(&h->c,&h->raw,(JxFarPointer){0x61de,h->c.data_segment},0,160);
                if(load_probe){h->input_probe_step=0;h->menu_probe=resume_walk?5:1;h->adaptive_skill_row=resume_slot;h->adaptive_skill_column=0;h->story_probe=1;}
                while(jx_load_game_menu(&h->c,(JxFarPointer){0,0x9d00})!=0){}
                h->menu_probe=0;h->story_probe=0;if(load_probe)key_scan(h,0x9c);
            } else {
                (void)JX_CALL(&h->c,0x1832,0x1884,0,0,320,200,0);
                jx_palette_write(&h->c,&h->raw,(JxFarPointer){0x61de,h->c.data_segment},0,160);
                jx_font_set_colors(&h->c,80,0,77);jx_font_set_mode(&h->c,176);
                jx_new_game_initialize(&h->c);
                h->story_probe=(unsigned)game_probe;
                if(jx_map_load(&h->c,jx_word(&h->c,0x9334),0,(JxFarPointer){0x300,0x9000}))fail(h,"game opening map stopped");
                h->story_probe=0;
            }
            if(scene_probe){
                h->story_probe=1;h->scripted_battles=1;h->story_poll_steps=0;
                jx_set_word(&h->c,0x9334,(uint16_t)place);
                if(jx_map_load(&h->c,(uint16_t)place,0,(JxFarPointer){0x300,0x9000}))fail(h,"scene fixture map failed");
                h->scripted_battles=1;
                printf("native true boot scene entry completed: place=%u polls=%u\n",place,h->story_poll_steps);
            }
            jx_image_tail_load(&h->c,(JxFarPointer){0x04e2,h->c.data_segment});
            if(jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"game scene stopped");
            goto world_ready;
        }
        if(argc>4 && (strcmp(argv[4],"--menu-probe")==0 || strcmp(argv[4],"--menu-down-probe")==0)) {
            jx_set_word(&h->c,0x04f2,0);jx_set_word(&h->c,0x0e5a,0);jx_set_word(&h->c,0x0868,1);
            jx_keyboard_initialize(&h->c,keyboard_bios,h);jx_timer_initialize(&h->c);
            jx_image_tail_load(&h->c,(JxFarPointer){0x04ca,h->c.data_segment});
            export_state(h,"main_decompiled/build/native_title_initial.bin");
            h->menu_probe=strcmp(argv[4],"--menu-down-probe")==0?2:1;
            unsigned expected=h->menu_probe==2?1:0,expected_steps=h->menu_probe==2?3:1;
            uint16_t selected=jx_two_choice_menu(&h->c,200,50,(JxFarPointer){0x04db,h->c.data_segment},(JxFarPointer){0x04d4,h->c.data_segment},(JxFarPointer){0,0x9a00});
            h->menu_probe=0;
            if(selected!=expected || h->input_probe_step!=expected_steps)fail(h,"title menu keyboard selection mismatch");
            key_scan(h,0x9c);
            export_state(h,"main_decompiled/build/native_title_final.bin");
            printf("native title menu scenario passed: Enter press/release selected=%u\n",selected);
        }
        jx_new_game_initialize(&h->c);
        if(argc>4 && strcmp(argv[4],"--newgame-probe")==0) {
            jx_set_word(&h->c,0x04f2,0);jx_set_word(&h->c,0x0e5a,0);jx_set_word(&h->c,0x0868,1);
            jx_keyboard_initialize(&h->c,keyboard_bios,h);jx_timer_initialize(&h->c);
            h->story_probe=1;
            if(jx_map_load(&h->c,jx_word(&h->c,0x9334),0,(JxFarPointer){0x300,0x9000}))fail(h,"new-game map stopped");
            h->story_probe=0;printf("new-game opening input polls=%u\n",h->story_poll_steps);
            if(jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"new-game scene stopped");
            JxFarPointer frame={0,0x9600};jx_game_loop_initialize(&h->c,frame);
            export_state(h,"main_decompiled/build/native_newgame_world_initial.bin");
            if(argc>5)key_scan(h,(uint8_t)strtoul(argv[5],NULL,0));
            for(unsigned tick=0;tick<64;tick++) {
                timer_tick(h);
                while(jx_game_loop_step(&h->c,frame)==JX_GAME_LOOP_PROCESSED)
                    if(!jx_world_state_bridge_can_step(&collision.state))fail(h,"new-game world divide fault");
            }
            export_state(h,"main_decompiled/build/native_newgame_world.bin");
            printf("new-game world scheduling completed: frames=%u\n",jx_word(&h->c,0x585e));
        }
        if(argc>3)export_state(h,argv[3]);
        printf("native boot initialization completed: chapter=%u place=%u heap_buffer=%04x EMS=%04x\n",jx_word(&h->c,0x8894),jx_word(&h->c,0x9334),jx_word(&h->c,0x2eb2),h->ems_frame);
        return 0;
    }
    if(scheduled)jx_graphics_initialize(&h->c);
    if(play) {
        h->window.video=h->video;h->window.palette=h->palette;h->window.opaque=h;h->window.legacy=&h->legacy_vga;h->window.mode=&h->legacy_mode;
        h->window.scan=key_scan;h->window.tick=timer_tick;
        if(!jx_native_window_open(&h->window))fail(h,"cannot open native display");
    }
    if(jx_map_load(&h->c,(uint16_t)place,0,(JxFarPointer){0x300,0x9000}))fail(h,"map load terminated");
    JxWorldUpdateFlow flow=jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100});
    if(flow){fprintf(stderr,"scene flow=%u\n",(unsigned)flow);fail(h,"scene drawing stopped");}
    if(h->video->error)fail(h,"unsupported VGA operation");
world_ready:
    if(item_description_probe){item_description_regression(h);return 0;}
    if(menu_system_probe || main_menu_probe){menu_regression(h);return 0;}
    if(resume_walk){
        h->adaptive_resume_slot=resume_slot;
        h->original_entry_probe=1;h->entry_world=1;h->entry_start=jx_word(&h->c,0x585e);
        h->walkthrough_phase=resume_phase;h->entry_ticks=0;h->menu_probe=0;h->story_probe=0;
        export_state(h,"main_decompiled/build/native_adaptive_resume_initial.bin");
        printf("native original save resume ready: phase=%u slot=%u map=%u position=(%u,%u)\n",resume_phase,resume_slot,jx_word(&h->c,0x9334),jx_word(&h->c,0x8760),jx_word(&h->c,0x8762));fflush(stdout);
        adaptive_load_route(h);
        (void)JX_CALL0(&h->c,0x1f2a,0xae40);
        fail(h,"resumed original game loop returned unexpectedly");
    }
    if(window_probe){
        jx_sound_resource_load(&h->c,75,0,(JxFarPointer){0x200,0x9000});
        if(jx_sound_play(&h->c,0)!=0)fail(h,"window audio fixture rejected resource");
    }
    export_ppm(h,"main_decompiled/build/native_scene.ppm");
    if(scheduled) {
        JxFarPointer frame={0,0x9600};jx_game_loop_initialize(&h->c,frame);
        if(!play)export_state(h,load_probe?"main_decompiled/build/native_loaded_world_initial.bin":game_probe?"main_decompiled/build/native_newgame_world_initial.bin":"main_decompiled/build/native_world_initial.bin");
        if(!play && argc>4)key_scan(h,(uint8_t)strtoul(argv[4],NULL,0));
        if(main_menu_probe){h->input_probe_step=0;h->menu_probe=3;key_scan(h,0x81);}
        if(disc_probe)jx_set_word(&h->c,0x585e,999);
        uint16_t starting_frame=jx_word(&h->c,0x585e);
        for(unsigned step=0;play?(ticks==0 || (uint16_t)(jx_word(&h->c,0x585e)-starting_frame)<ticks):step<ticks;step++) {
            if(play){if(!jx_native_window_pump(&h->window))window_shutdown(h);}
            else timer_tick(h);
            while(jx_game_loop_step(&h->c,frame)==JX_GAME_LOOP_PROCESSED)
                if(!jx_world_state_bridge_can_step(&collision.state)) {
                    fprintf(stderr,"guest divide fault %04x:%04x flow=%u\n",collision.state.fault_segment,
                        collision.state.fault_offset,(unsigned)collision.state.flow);
                    fail(h,"world update halted at a guest divide fault");
                }
        }
        if(main_menu_probe){h->menu_probe=0;if(h->input_probe_step!=2)fail(h,"main menu cancel input mismatch");printf("main menu Escape cancel scenario passed\n");}
        if(!window_probe)jx_native_window_close(&h->window);
        if(play) {
            if(!h->window.paints || (ticks && h->window.ticks<ticks))fail(h,"native display did not complete scheduled paints/timers");
            printf("native window: paints=%u timer_messages=%u\n",h->window.paints,h->window.ticks);
            FILE *report=fopen("main_decompiled/analysis/native_window_verification.json","wb");
            if(!report)fail(h,"cannot create window report");
            fprintf(report,"{\"status\":\"PASS\",\"place\":%u,\"paints\":%u,\"timer_messages\":%u,\"world_frames\":%u,\"scope\":\"Native Win32 window paints and real WM_TIMER to recovered IRQ/queue/main loop. No GUI keyboard interaction asserted.\"}\n",
                place,h->window.paints,h->window.ticks,jx_word(&h->c,0x585e));
            fclose(report);
        }
        printf("native world scheduling completed: frames=%u\n",jx_word(&h->c,0x585e));
    }
    if(sound_probe){
        for(unsigned resource_number=0;resource_number<2000;resource_number++){
            char name[64];snprintf(name,sizeof name,"VOC/JX%02u.VOC",resource_number);
            FILE *asset=fopen(name,"rb");if(!asset)continue;fclose(asset);
            jx_sound_resource_load(&h->c,(uint16_t)resource_number,0,(JxFarPointer){0x200,0x9000});
            if(jx_sound_play(&h->c,0)!=0)fail(h,"native sound resource playback rejected");
        }
        printf("native PCM resource exports completed: %u\n",h->audio_exports);
    }
    if(pickup_probe){
        uint16_t item=jx_word(&h->c,0x92be),amount=jx_word(&h->c,0x92c0);
        uint16_t state=(uint16_t)(0xacec+10*jx_word(&h->c,0x9334));
        if(!item || jx_ds(&h->c)[state])fail(h,"pickup fixture requires original available object0");
        jx_set_word(&h->c,0x8760,jx_word(&h->c,0x92b8));
        jx_set_word(&h->c,0x8762,(uint16_t)(jx_word(&h->c,0x92ba)+1));jx_set_word(&h->c,0x8764,0);
        if(jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"pickup fixture redraw failed");
        export_state(h,"main_decompiled/build/native_pickup_initial.bin");
        h->story_probe=1;h->story_poll_steps=0;
        jx_party_interact(&h->c);jx_party_interact(&h->c);
        h->story_probe=0;
        export_state(h,"main_decompiled/build/native_pickup_final.bin");
        uint16_t count=0;for(unsigned slot=0;slot<64;slot++)if(jx_word(&h->c,(uint16_t)(0x878a+slot*4))==item)count=jx_word(&h->c,(uint16_t)(0x878c+slot*4));
        if(count!=amount || jx_ds(&h->c)[state]!=1)fail(h,"pickup or duplicate prevention mismatch");
        printf("native actual pickup twice PASS: item=%u count=%u polls=%u\n",item,count,h->story_poll_steps);
    }
    if(use_item_probe){
        jx_set_word(&h->c,0xd05e,170);jx_set_word(&h->c,0xd060,5);
        export_state(h,"main_decompiled/build/native_use_item_initial.bin");
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        jx_party_use_item(&h->c);
        h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_use_item_final.bin");
        if(jx_word(&h->c,0x878a)!=50 || jx_word(&h->c,0x878c)!=1 || jx_word(&h->c,0xd05e)!=190 || jx_word(&h->c,0xd060)!=20)fail(h,"actual item use fixture mismatch");
        printf("native actual item use PASS: item=50 remaining=1 HP=190 MP=20 polls=%u\n",h->story_poll_steps);
    }
    if(board_probe){
        if(board_window_probe){
            h->window.video=h->video;h->window.palette=h->palette;h->window.opaque=h;
            h->window.legacy=&h->legacy_vga;h->window.mode=&h->legacy_mode;
            h->window.scan=key_scan;h->window.tick=timer_tick;h->window_probe=1;
            if(!jx_native_window_open(&h->window))fail(h,"cannot open board display");
        }
        export_state(h,"main_decompiled/build/native_board_initial.bin");
        h->memory[0x9fe00]=9;h->memory[0x9fe01]=0;h->memory[0x9fe02]=9;h->memory[0x9fe03]=0;h->memory[0x9fe04]=0;
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        int resource_id=argc>3?atoi(argv[3]):0;
        if(resource_id<0 || resource_id>3)fail(h,"invalid board probe resource");
        h->board_probe_mode=1;
        int result=(int16_t)JX_CALL(&h->c,0x17d2,0x01da,resource_id);
        h->board_probe_mode=0;
        h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_board_final.bin");
        if(board_window_probe && !h->window.planar_paints)fail(h,"no native planar paints");
        if(result || h->legacy_mode!=0x13)fail(h,"board did not solve/restore ModeX");
        printf("native actual board PASS: resource=%d result=%d input/delay polls=%u planar_paints=%u\n",resource_id,result,h->story_poll_steps,h->window.planar_paints);
    }
    if(dice_probe){
        export_state(h,"main_decompiled/build/native_dice_initial.bin");
        uint32_t money=jx_word(&h->c,0x8756)|((uint32_t)jx_word(&h->c,0x8758)<<16);
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        uint16_t result=(uint16_t)JX_CALL(&h->c,0x1f2a,0xb6a0,1,1,0);
        h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_dice_final.bin");
        uint32_t after=jx_word(&h->c,0x8756)|((uint32_t)jx_word(&h->c,0x8758)<<16);
        if(result>1 || after!=(result?money+1:money-1))fail(h,"actual dice settlement mismatch");
        printf("native dice round PASS: outcome=%u money=%u->%u polls=%u\n",result,money,after,h->story_poll_steps);
    }
    if(fixed_animation_probe){
        static const uint16_t names[]={0x101c,0x1028,0x1080,0x10a7,0x1222};
        static const uint16_t delays[]={300,300,200,200,500};
        unsigned variant=argc>3?(unsigned)strtoul(argv[3],NULL,10):4;
        if(variant>=5)fail(h,"fixed animation variant must be0..4");
        char path[128];snprintf(path,sizeof path,"main_decompiled/build/native_fixed_animation_%u_initial.bin",variant);
        export_state(h,path);
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        (void)JX_CALL(&h->c,0x2f28,0x6f12,names[variant],h->c.data_segment,delays[variant],variant==4);
        h->story_probe=0;h->battle_input=0;
        snprintf(path,sizeof path,"main_decompiled/build/native_fixed_animation_%u_final.bin",variant);
        export_state(h,path);
        printf("native actual fixed animation PASS: variant=%u frames=%u polls=%u\n",variant,jx_word(&h->c,0x3548),h->story_poll_steps);
    }
    if(cinematic_probe){
        unsigned variant=argc>3?(unsigned)strtoul(argv[3],NULL,10):0;
        if(variant>1)fail(h,"cinematic variant must be0 or1");
        export_state(h,"main_decompiled/build/native_cinematic_initial.bin");
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        JxFlow flow=jx_cinematic_180(&h->c,(JxFarPointer){variant?0x123a:0x121a,h->c.data_segment},
            (JxFarPointer){variant?0x122e:0x1212,h->c.data_segment},input_poll,h);
        h->story_probe=0;h->battle_input=0;
        if(flow!=JX_FLOW_RETURNED || h->story_poll_steps<2501)fail(h,"cinematic did not complete2501 timer events");
        export_state(h,"main_decompiled/build/native_cinematic_final.bin");
        printf("native actual cinematic PASS: variant=%u polls=%u\n",variant,h->story_poll_steps);
    }
    /* Explicit strong-party test fixture; battle records, controller,
     * damage, animation, menus and settlement still execute unchanged. */
    if(final_battles_probe || ending_survivor_probe){
        for(unsigned actor=0;actor<2;actor++){

            uint16_t base=(uint16_t)(0xd04e + actor*0x44);
            for(unsigned field=0x10;field<=0x1a;field+=2)jx_set_word(&h->c,(uint16_t)(base+field),30000);
        }
    }
    if(final_death_probe){
        for(unsigned actor=0;actor<2;actor++){

            uint16_t base=(uint16_t)(0xd04e + actor*0x44);
            jx_set_word(&h->c,(uint16_t)(base+0x10),1);
            jx_set_word(&h->c,(uint16_t)(base+0x18),0);
            jx_set_word(&h->c,(uint16_t)(base+0x1a),0);
        }
    }
    if(final_battles_probe || final_death_probe){
        export_state(h,final_death_probe?"main_decompiled/build/native_final_death_initial.bin":"main_decompiled/build/native_final_battles_initial.bin");
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        (void)JX_CALL0(&h->c,0x1512,0x000c);
        h->story_probe=0;h->battle_input=0;
        if(final_death_probe){
            FILE *saved=fopen("SAVE0.DAT","rb");uint8_t raw[1406];
            if(!saved || fread(raw,1,sizeof raw,saved)!=sizeof raw || fclose(saved))fail(h,"cannot inspect original save0 fixture");
            uint16_t saved_place=(uint16_t)(raw[1404]|((uint16_t)raw[1405]<<8));
            if(jx_word(&h->c,0x9334)!=saved_place || jx_word(&h->c,0xb684)!=0)fail(h,"final defeat did not return through original save load");
        }else if(jx_ds(&h->c)[0x7f6f] || jx_ds(&h->c)[0x7f6e] || jx_ds(&h->c)[0x7f70] || jx_ds(&h->c)[0x7f71])fail(h,"final battle chain did not defeat all four enemies");
        export_state(h,final_death_probe?"main_decompiled/build/native_final_death_final.bin":"main_decompiled/build/native_final_battles_final.bin");
        printf("native final actual battle chain PASS: defeat_load=%d place=%u polls=%u\n",final_death_probe,jx_word(&h->c,0x9334),h->story_poll_steps);
    }
    if(ending_probe){
        jx_ds(&h->c)[0xd04d]=0;
        if(ending_reunion_probe)jx_ds(&h->c)[0xce8b]=0;
        if(ending_survivor_probe)jx_ds(&h->c)[0xce8b]=1;
        export_state(h,"main_decompiled/build/native_ending_initial.bin");
        h->cutscene_fixture=1;h->credits_limit=205;h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        if(ending_reunion_probe || ending_survivor_probe)(void)JX_CALL(&h->c,0x2f28,0x34f4,4);
        else if(ending_walk_probe)(void)JX_CALL0(&h->c,0x2f28,0x2946);
        else if(ending_film_probe)(void)JX_CALL0(&h->c,0x2f28,0x2a46);
        else (void)JX_CALL(&h->c,0x2f28,0x2b58,0);
        fail(h,"ending did not execute original shutdown");
    }
    if(ending_animation_probe){
        export_state(h,"main_decompiled/build/native_ending_animation_initial.bin");
        h->cutscene_fixture=2;h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        (void)JX_CALL(&h->c,0x2f28,0x3b8e,0x1244,h->c.data_segment);
        h->cutscene_fixture=0;h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_ending_animation_final.bin");
        if(h->story_poll_steps<281)fail(h,"ending animation did not complete its original duration");
        printf("native ending ANM118 PASS: frames=%u polls=%u\n",jx_word(&h->c,0x3548),h->story_poll_steps);
    }
    if(credits_probe){
        h->credits_limit=argc>4?(unsigned)strtoul(argv[4],NULL,0):3500;
        export_state(h,"main_decompiled/build/native_credits_initial.bin");
        h->cutscene_fixture=1;h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        (void)JX_CALL(&h->c,0x2f28,0x7ec8);
        h->cutscene_fixture=0;h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_credits_final.bin");
        if((h->credits_limit>=3500 && h->credits_position!=3399) || h->credits_frames<h->credits_limit)fail(h,"credits did not scroll through all original images");
        printf("native credits PASS: frames=%u position=%u polls=%u\n",h->credits_frames,h->credits_position,h->story_poll_steps);
    }
    if(menus_probe){
        export_state(h,"main_decompiled/build/native_menus_initial.bin");
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        (void)JX_CALL(&h->c,0x1f2a,0x864a,0);
        uint16_t skill=(uint16_t)JX_CALL(&h->c,0x1f2a,0x9952,0);
        uint16_t item=(uint16_t)JX_CALL(&h->c,0x1f2a,0x9182);
        h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_menus_final.bin");
        printf("native real actor/skill/item menus completed: polls=%u skill=%u item=%u\n",h->story_poll_steps,skill,item);
    }
    if(board_quest_probe){
        if(place!=194 || jx_word(&h->c,0x88e6)!=0x2ce4 || jx_word(&h->c,0x88e8)!=0x2f28)fail(h,"actual board NPC callback is not installed");
        jx_set_word(&h->c,0x8760,jx_word(&h->c,0x889c+64));
        jx_set_word(&h->c,0x8762,(uint16_t)(jx_word(&h->c,0x889c+66)+1));jx_set_word(&h->c,0x8764,0);
        if(jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"board NPC scene redraw");
        export_state(h,"main_decompiled/build/native_board_quest_initial.bin");
        h->story_probe=1;h->battle_input=1;h->board_probe_mode=1;h->story_poll_steps=0;
        for(unsigned challenge=0;challenge<5;++challenge){
            unsigned state=0xcd42;if(jx_ds(&h->c)[state]!=challenge)fail(h,"board NPC state did not advance");
            jx_party_interact(&h->c);
        }
        h->story_probe=0;h->battle_input=0;h->board_probe_mode=0;
        export_state(h,"main_decompiled/build/native_board_quest_final.bin");
        if(jx_ds(&h->c)[0xcd42]!=6 || !jx_inventory_has(&h->c,45))fail(h,"board NPC reward not completed");
        printf("native actual board NPC quest PASS: challenges=4 state=6 item45=%u polls=%u\n",jx_inventory_has(&h->c,45),h->story_poll_steps);
    }
    if(npc_callback_probe){
        unsigned npc=argc>3?(unsigned)strtoul(argv[3],NULL,10):0;
        unsigned state=argc>4?(unsigned)strtoul(argv[4],NULL,10):0;
        unsigned kind=trigger_callback_probe?3:contact_callback_probe?2:argc>6?(unsigned)strtoul(argv[6],NULL,10):0;
        if(npc>=(trigger_callback_probe?9u:30u) || state>255 || (!trigger_callback_probe && kind>2))fail(h,"invalid NPC callback fixture");
        uint16_t map=jx_word(&h->c,0x9334);
        uint16_t field=trigger_callback_probe?(uint16_t)(0x6584+8*npc+4):(uint16_t)(0x889c+86*npc+74+4*kind);
        uint16_t off=jx_word(&h->c,field),seg=jx_word(&h->c,(uint16_t)(field+2));
        if((!trigger_callback_probe && jx_ds(&h->c)[(uint16_t)(0x665a+30*map+npc)]!=1) || (!off && !seg) || (seg==0x2f28 && off==0x86c4)){
            printf("native NPC callback SKIP: map=%u npc=%u kind=%u target=%04x:%04x\n",map,npc,kind,seg,off);
        }else{
            if(!trigger_callback_probe)jx_ds(&h->c)[(uint16_t)(0xb686+30*map+npc)]=(uint8_t)state;
            if(argc>7 && strcmp(argv[7],"--strong-party")==0){
                for(unsigned actor=0;actor<2;++actor){
                    uint16_t base=(uint16_t)(0xd04e + actor*0x44);
                    for(unsigned stat=0x10;stat<=0x1a;stat+=2)jx_set_word(&h->c,(uint16_t)(base+stat),30000);
                }
            }else if(argc>8){
                unsigned gate=(unsigned)strtoul(argv[7],NULL,10),mask=(unsigned)strtoul(argv[8],NULL,10);
                if(map!=22 || npc<10 || npc>13 || gate>1 || mask>15)fail(h,"invalid map22 switch fixture");
                jx_ds(&h->c)[0x68fc]=(uint8_t)gate;
                for(unsigned sw=0;sw<4;++sw)jx_set_word(&h->c,(uint16_t)(0x889c+86*(10+sw)+60),(uint16_t)((mask>>(3-sw))&1));
            }
            if(contact_callback_probe){
                uint16_t position=trigger_callback_probe?(uint16_t)(0x6584+8*npc):(uint16_t)(0x889c+86*npc+64);
                jx_set_word(&h->c,0x8760,jx_word(&h->c,position));
                jx_set_word(&h->c,0x8762,jx_word(&h->c,(uint16_t)(position+2)));
                if(walk_contact_probe){
                    unsigned direction=argc>6?(unsigned)strtoul(argv[6],NULL,10):3;
                    if(direction>3)fail(h,"invalid walk contact direction");
                    if(!trigger_callback_probe){
                        uint16_t actor=(uint16_t)(0x889c+86*npc),descriptor=(uint16_t)(0x31ce + 18*npc);
                        int active=jx_word(&h->c,(uint16_t)(actor+0x1a))!=0;
                        uint16_t width=active?(uint16_t)(jx_word(&h->c,descriptor)+2):5;
                        uint16_t height=active?(uint16_t)(jx_word(&h->c,(uint16_t)(descriptor+2))+2):5;
                        uint16_t left=(uint16_t)(jx_word(&h->c,position)-(active?1:2));
                        uint16_t top=(uint16_t)(jx_word(&h->c,(uint16_t)(position+2))-(active?jx_word(&h->c,(uint16_t)(descriptor+2)):2));
                        jx_set_word(&h->c,0x8760,(direction==1 || direction==2)?(uint16_t)(left+width-1):left);
                        jx_set_word(&h->c,0x8762,direction>=2?(uint16_t)(top+height/2):direction==1?top:(uint16_t)(top+height-1));
                    }
                    h->contact_expected_x=jx_word(&h->c,0x8760);h->contact_expected_y=jx_word(&h->c,0x8762);
                    uint16_t coordinate=direction<2?0x8762:0x8760;
                    jx_set_word(&h->c,coordinate,(uint16_t)(jx_word(&h->c,coordinate)+((direction&1)?-1:1)));
                    uint16_t period=jx_word(&h->c,0x5860);
                    if(!period)fail(h,"walk contact zero movement period");
                    jx_set_word(&h->c,0x585e,(uint16_t)(jx_word(&h->c,0x585e)/period*period));
                    if(world_contact_probe){
                        jx_set_word(&h->c,0x585e,(uint16_t)(jx_word(&h->c,0x585e)-1));
                        jx_game_loop_initialize(&h->c,(JxFarPointer){0,0x9600});
                    }
                    static const uint8_t scans[]={0x48,0x50,0x4b,0x4d};
                    static const uint8_t releases[]={0xc8,0xd0,0xcb,0xcd};
                    h->contact_release=releases[direction];
                    key_scan(h,scans[direction]);
                }
                if(jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"contact fixture redraw failed");
            }
            export_state(h,"main_decompiled/build/native_npc_callback_initial.bin");
            h->story_probe=1;h->battle_input=world_contact_probe?0:1;h->scripted_battles=1;h->story_poll_steps=0;
            printf("native NPC callback BEGIN: map=%u npc=%u state=%u kind=%u target=%04x:%04x\n",map,npc,state,kind,seg,off);fflush(stdout);
            if(contact_callback_probe){
                h->contact_seg=seg;h->contact_off=off;h->contact_hits=0;
                h->contact_walk=(unsigned)walk_contact_probe;
                uint16_t start_x=jx_word(&h->c,0x8760),start_y=jx_word(&h->c,0x8762);
                if(world_contact_probe){
                    unsigned ticks=0,steps=0;
                    h->scheduled_contact=1;
                    while(!h->contact_hits && ticks<256){
                        timer_tick(h);++ticks;
                        while(!h->contact_hits && jx_game_loop_step(&h->c,(JxFarPointer){0,0x9600})==JX_GAME_LOOP_PROCESSED){
                            if(++steps>4096)fail(h,"scheduled contact step limit exceeded");
                            if(!jx_world_state_bridge_can_step(&collision.state))fail(h,"scheduled contact world divide fault");
                        }
                    }
                    h->scheduled_contact=0;
                    printf("native scheduled world contact ticks=%u processed=%u hits=%u\n",ticks,steps,h->contact_hits);
                }else (void)JX_CALL0(&h->c,0x2f28,walk_contact_probe?0x8fd4:0x88de);
                h->contact_seg=0;
                h->contact_walk=0;
                if(!h->contact_hits){
                    if(walk_contact_probe)export_state(h,"main_decompiled/build/native_npc_callback_final.bin");
                    fail(h,"requested callback not reached by actual contact logic");
                }
                printf("native actual contact callback hits=%u\n",h->contact_hits);
                if(walk_contact_probe){
                    static const uint8_t releases[]={0xc8,0xd0,0xcb,0xcd};
                    unsigned direction=argc>6?(unsigned)strtoul(argv[6],NULL,10):3;
                    if(!world_contact_probe)key_scan(h,releases[direction]);
                    printf("native actual player movement contact PASS: direction=%u start=(%u,%u) final=(%u,%u)\n",direction,start_x,start_y,jx_word(&h->c,0x8760),jx_word(&h->c,0x8762));
                }
            }else if(trigger_callback_probe)(void)JX_CALL0(&h->c,seg,off);
            else (void)JX_CALL(&h->c,seg,off,(uint16_t)npc);
            h->story_probe=0;h->battle_input=0;
            export_state(h,"main_decompiled/build/native_npc_callback_final.bin");
            printf("native NPC callback PASS: map=%u npc=%u state=%u kind=%u target=%04x:%04x final_map=%u polls=%u\n",map,npc,state,kind,seg,off,jx_word(&h->c,0x9334),h->story_poll_steps);
        }
    }
    if(interact_probe){
        jx_set_word(&h->c,0x8760,jx_word(&h->c,0x889c+86*2+64));
        jx_set_word(&h->c,0x8762,(uint16_t)(jx_word(&h->c,0x889c+86*2+66)+1));
        jx_set_word(&h->c,0x8764,0);
        if(jx_scene_redraw_world(&h->c,(JxFarPointer){0,0x9100}))fail(h,"interaction fixture scene failed");
        export_state(h,"main_decompiled/build/native_interact_initial.bin");
        h->story_probe=1;h->story_poll_steps=0;
        jx_party_interact(&h->c);
        h->story_probe=0;
        export_state(h,"main_decompiled/build/native_interact_final.bin");
        if(jx_ds(&h->c)[0xb688]!=1)fail(h,"real NPC interaction did not advance state");
        printf("native actual NPC interaction PASS: npc=2 state=%u polls=%u\n",jx_ds(&h->c)[0xb688],h->story_poll_steps);
    }
    if(story_continuation_probe){
        jx_ds(&h->c)[0xb686]=5;
        export_state(h,"main_decompiled/build/native_story_continuation_initial.bin");
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        jx_event_005_0594(&h->c,0);
        h->story_probe=0;h->battle_input=0;
        export_state(h,"main_decompiled/build/native_story_continuation_final.bin");
        printf("native opening story continuation completed: polls=%u flag3469=%u inventory18=%u\n",h->story_poll_steps,jx_ds(&h->c)[0x3469],jx_inventory_has(&h->c,18));
    }
    if(battle_probe){
        export_state(h,"main_decompiled/build/native_battle_initial.bin");
        h->story_probe=1;h->battle_input=1;h->story_poll_steps=0;
        uint16_t result=jx_battle_run(&h->c,1,0,0xffff,(JxFarPointer){0,0xb000});
        h->story_probe=0;h->battle_input=0;export_state(h,"main_decompiled/build/native_battle_final.bin");
        printf("native battle completed: result=%u polls=%u\n",result,h->story_poll_steps);
        FILE *report=fopen("main_decompiled/analysis/native_battle_integration.json","wb");
        if(!report)fail(h,"cannot write native battle integration report");
        fprintf(report,"{\"status\":\"PASS\",\"encounter\":1,\"actor\":0,\"result\":%u,\"polls\":%u,\"turns\":%u,\"enemy_hp\":%u,\"scope\":\"Native recovered battle controller, real resource loaders, menus, turns, rendering, settlement and return to world. Explicit finite Enter IRQ/timer schedule; CD and Sound Blaster disabled. This run is not an independent whole original battle comparison.\"}\n",result,h->story_poll_steps,jx_word(&h->c,0x2932),jx_word(&h->c,0x6614));
        fclose(report);
    }
    if(window_probe){
        if(h->cd.image && !h->cd.audio.plays)fail(h,"original CD audio was not submitted");
        printf("native CD waveOut submissions=%u plays=%u\n",h->cd.audio.plays,h->cd.plays);
        if(!h->audio.plays || !h->audio_irqs || !h->window.paints || h->window.ticks<64)fail(h,"whole window/audio fixture incomplete");
        printf("native whole window probe PASS: audio submissions=%u IRQs=%u paints=%u timers=%u\n",h->audio.plays,h->audio_irqs,h->window.paints,h->window.ticks);
        FILE *report=fopen("main_decompiled/analysis/native_game_window_audio_verification.json","wb");
        if(!report)fail(h,"cannot write whole window report");
        fprintf(report,"{\"status\":\"PASS\",\"audio_submissions\":%u,\"paints\":%u,\"timer_messages\":%u,\"scope\":\"Real Win32 title and full opening with bounded posted keyboard messages, recovered IRQ/input queue, 64 world frames, original VOC75 through waveOut PCM device, original shutdown. waveOut acceptance verified; human audibility not asserted.\"}\n",h->audio.plays,h->window.paints,h->window.ticks);
        fclose(report);
        if(!jx_native_window_post_scan(&h->window,0x4d) || !jx_native_window_pump(&h->window) || !h->window.pressed[0x4d])fail(h,"window keydown fixture failed");
        if(!jx_native_window_post_focus_loss(&h->window) || !jx_native_window_pump(&h->window) || h->window.pressed[0x4d])fail(h,"window focus release fixture failed");
        printf("native window focus release PASS\n");
        if(argc>3 && strcmp(argv[3],"--close")==0){
            if(!jx_native_window_post_close(&h->window))fail(h,"cannot post close fixture");
            if(jx_native_window_pump(&h->window))fail(h,"close fixture did not close");
            window_shutdown(h);fail(h,"window close cleanup returned");
        }
        h->story_probe=1;jx_game_shutdown(&h->c);fail(h,"window fixture shutdown unexpectedly returned");
    }
    if(shutdown_probe){h->story_probe=1;jx_game_shutdown(&h->c);fail(h,"shutdown probe unexpectedly returned");}
    if(save_probe) {
        export_state(h,"main_decompiled/build/native_save_initial.bin");
        h->input_probe_step=0;h->menu_probe=1;
        jx_save_game_menu(&h->c,(JxFarPointer){0,0x9d00});
        h->menu_probe=0;key_scan(h,0x9c);
        export_state(h,"main_decompiled/build/native_save_final.bin");
        printf("native save menu completed: isolated userdata/SAVE0.DAT\n");
    }
    if(game_probe && !save_probe)export_state(h,load_probe?"main_decompiled/build/native_loaded_world.bin":"main_decompiled/build/native_newgame_world.bin");
    if(argc>2 && !play && !game_probe)export_state(h,argv[2]);
    if(game_probe && argc>3 && (strcmp(argv[2],"--game-probe")==0 || strcmp(argv[2],"--load-probe")==0))export_state(h,argv[3]);
    printf("native resource scene PASS: place=%u host_calls=%u EMS_maps=%u files=%u\n",jx_word(&h->c,0x9334),h->calls,h->maps,h->next_file);
    return 0;
}
