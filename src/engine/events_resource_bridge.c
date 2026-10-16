#include <assert.h>
#include "event_engine.h"
static uint32_t resource_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *a,size_t n) {
    JxResourceBridge *b=(JxResourceBridge *)opaque;JxEventContext *c=b->base.context;
    if(segment==0x2f28 && offset==0x6dfe) {assert(n==2);jx_image_background_present(c,(JxFarPointer){a[0],a[1]});return 0;}
    if(segment==0x2f28 && offset==0x7c44) {assert(n==0);jx_credits_cache(c);return 0;}
    if(segment==0x2f28 && offset==0x7d4e) {assert(n==1);jx_credits_scroll_frame(c,a[0]);return 0;}
    if(segment==0x2f28 && offset==0x6d18) {
        assert(n==2);jx_image_present(c,(JxFarPointer){a[0],a[1]});return 0;
    }
    if(segment==0x1f2a) switch(offset) {
    case 0x58fa:assert(n==2);jx_image_tail_load(c,(JxFarPointer){a[0],a[1]});return 0;
    case 0x155c:assert(n==1);return jx_sound_play(c,a[0]);
    case 0x2820:assert(n==1);return jx_sound_dsp_reset(c,a[0]);
    case 0x288a:assert(n==7);return jx_animation_crop_cache(c,(JxFarPointer){a[0],a[1]},a[2],a[3],a[4],a[5],a[6]);
    case 0x0000:assert(n==0);jx_font_resources_load(c);return 0;
    case 0xc7f2:assert(n==0);jx_npc_reset(c);return 0;
    case 0xc9a0:assert(n==0);jx_scene_objects_reset(c);return 0;
    case 0xc6a8:assert(n==0);jx_scene_sprites_load(c);return 0;
    case 0xc264:assert(n==2);jx_sprite_load(c,a[0],a[1],b->sprite);return 0;
    case 0x640e:assert(n==1);return jx_scene_objects_load(c,a[0]);
    case 0x65fc:assert(n==0);jx_scene_backdrop_load(c,b->archive);return 0;
    case 0x6496:assert(n==1);return jx_place_load(c,a[0]);
    case 0x5230:assert(n==4);return jx_archive_open(c,(JxFarPointer){a[0],a[1]},(JxFarPointer){a[2],a[3]},b->archive);
    case 0x5970:assert(n==3);return jx_animation_cache(c,(JxFarPointer){a[0],a[1]},a[2]);
    case 0x5ae8:assert(n==0);jx_weather_load(c);return 0;
    case 0x5fd8:assert(n==2);jx_sound_resource_load(c,a[0],a[1],b->sound);return 0;
    default:break;
    }
    return b->base.fallback(b->base.fallback_opaque,segment,offset,a,n);
}
void jx_resource_bridge_install(JxEventContext *c,JxResourceBridge *b,JxFarPointer sprite,JxFarPointer archive,JxFarPointer sound) {
    assert(c && b && c->call && c->call!=resource_dispatch);
    b->base.context=c;b->base.fallback=c->call;b->base.fallback_opaque=c->opaque;
    b->sprite=sprite;b->archive=archive;b->sound=sound;c->call=resource_dispatch;c->opaque=b;
}
void jx_resource_bridge_remove(JxEventContext *c,JxResourceBridge *b) {
    assert(c->call==resource_dispatch && c->opaque==b);
    c->call=b->base.fallback;c->opaque=b->base.fallback_opaque;
}
