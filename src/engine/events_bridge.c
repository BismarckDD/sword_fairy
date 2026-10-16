#include "event_engine.h"
#include <assert.h>
#include <string.h>

static JxFarPointer far_argument(const uint16_t *args) {
    JxFarPointer p={args[0],args[1]};return p;
}

static uint32_t inventory_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    JxEngineBridge *b=opaque;JxEventContext *c=b->context;
    if(segment==0x1f2a) {
        switch(offset) {
        case 0x747c:assert(count==1);jx_actor_restore_current(c,args[0]);return 0;
        case 0x8ca8:assert(count==1);return jx_inventory_has(c,args[0]);
        case 0x8bd6:assert(count==0);jx_inventory_compact(c);return 0;
        case 0x8f28:assert(count==2);jx_inventory_remove(c,args[0],args[1]);return 0;
        case 0x8e1c:assert(count==1);jx_actor_skills_compact(c,args[0]);return 0;
        case 0x8ec4:assert(count==2);jx_actor_skill_remove(c,(int16_t)jx_signed_word(args[0]),args[1]);return 0;
        case 0x966e:assert(count==2);return jx_actor_skill_grant(c,args[0],args[1]);
        default:break;
        }
    }
    return b->fallback(b->fallback_opaque,segment,offset,args,count);
}
void jx_inventory_bridge_install(JxEventContext *c,JxEngineBridge *b) {
    assert(c && b && c->call && c->call!=inventory_dispatch);
    b->context=c;b->fallback=c->call;b->fallback_opaque=c->opaque;
    c->call=inventory_dispatch;c->opaque=b;
}

/* Pure memory-side graphics operations; no VGA or EMS service is faked. */
static uint32_t graphics_memory_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    JxEngineBridge *bridge=opaque;JxEventContext *c=bridge->context;
    if(segment==0x1832 && offset==0x185e) {
        assert(count==6);jx_memory_copy_transparent(c,far_argument(args),far_argument(args+2),args[4],args[5]);return 0;
    }
    if(segment==0x2f28) {
        switch(offset) {
        case 0x6c7e:
            assert(count==7);jx_color_map_pixels(c,far_argument(args),far_argument(args+2),args[4],far_argument(args+5));return 0;
        case 0xa8d0: {
            assert(count==8);
            uint32_t bits=args[6]|((uint32_t)args[7]<<16);float weight;
            memcpy(&weight,&bits,sizeof(weight));
            return (uint16_t)jx_palette_build_map(c,far_argument(args),args[2]|((uint32_t)args[3]<<16),far_argument(args+4),weight);
        }
        case 0xa72e:
            assert(count==4);return (uint16_t)jx_palette_nearest_exact(c,(uint8_t)args[0],(uint8_t)(args[0]>>8),(uint8_t)args[1],far_argument(args+2));
        case 0xa7fe:
            assert(count==5);return (uint16_t)jx_palette_nearest(c,(uint8_t)args[0],(uint8_t)(args[0]>>8),(uint8_t)args[1],far_argument(args+2),(int16_t)jx_signed_word(args[4]));
        default:break;
        }
    }
    return bridge->fallback(bridge->fallback_opaque,segment,offset,args,count);
}

/* A separately installable CRT layer lets the host combine recovered code
 * with the scene bridge without changing its opaque state or ABI. */
static uint32_t runtime_dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    JxEngineBridge *bridge=(JxEngineBridge *)opaque;
    JxEventContext *c=bridge->context;
    if(segment==0x1832 && (offset==0x2832 || offset==0x2864 || offset==0x28c4)) {
        assert(count==4);uint32_t left=args[0]|((uint32_t)args[1]<<16),right=args[2]|((uint32_t)args[3]<<16),value;
        if(offset==0x2832)return jx_module_multiply_u32(left,right);
        int valid=offset==0x2864?jx_divide_unsigned32(left,right,&value):jx_remainder_unsigned32(left,right,&value);
        if(valid)return value;
        return bridge->fallback(bridge->fallback_opaque,segment,offset,args,count);
    }
    if(segment==0x1000) {
        switch(offset) {
        case 0x1ef6:
            assert(count==2);return jx_getc(c,args[0]);
        case 0x0ada:
            assert(count==2);return jx_stream_refill(c,args[0]);
        case 0x3146:case 0x3514:
            assert(count==5);return jx_memory_copy(c,far_argument(args),far_argument(args+2),args[4]);
        case 0x30ea:
            assert(count==5);return (uint16_t)jx_memory_compare(c,far_argument(args),far_argument(args+2),args[4]);
        case 0x3384: {
            uint32_t quotient;assert(count==4);
            if(jx_divide_signed32(args[0]|((uint32_t)args[1]<<16),args[2]|((uint32_t)args[3]<<16),&quotient)) return quotient;
            break;
        }
        case 0x31a4:case 0x3572:
            assert(count==4);return jx_memory_fill(c,far_argument(args),args[2],args[3]);
        case 0x2448:
            assert(count==4);return jx_string_copy(c,far_argument(args),far_argument(args+2));
        case 0x24ae:
            assert(count==2);return jx_string_length(c,far_argument(args));
        case 0x23f4:
            assert(count==4);return jx_string_append(c,far_argument(args),far_argument(args+2));
        case 0x341e:
            assert(count==4);return jx_multiply_u32(args[0]|((uint32_t)args[1]<<16),args[2]|((uint32_t)args[3]<<16));
        case 0x32a0:
            assert(count==1);return jx_random_seed(c,args[0]);
        case 0x32b2: {
            uint16_t value;assert(count==0);value=jx_random_next(c);
            return value|((uint32_t)jx_word(c,0x1754)<<16);
        }
        default:break;
        }
    }
    return bridge->fallback(bridge->fallback_opaque,segment,offset,args,count);
}

static uint32_t dispatch(void *opaque,uint16_t segment,uint16_t offset,const uint16_t *args,size_t count) {
    JxEngineBridge *bridge=(JxEngineBridge *)opaque;
    JxEventContext *c=bridge->context;
    if(segment==0x2f28) {
        switch(offset) {
        case 0x56cc:assert(count==0);jx_camera_lock(c,0);return 0;
        case 0x56dc:assert(count==0);jx_camera_unlock(c,0);return 0;
        case 0x5580:assert(count==0);jx_scene_redraw(c,0);return 0;
        case 0x4fdc:assert(count==2);jx_scene_draw_tile(c,(int16_t)jx_signed_word(args[0]),(int16_t)jx_signed_word(args[1]));return 0;
        case 0x4384:assert(count==2);return jx_tile_map_resource(c,args[0],args[1]);
        case 0x56ec:assert(count==2);jx_camera_scroll(c,args[0],(int16_t)jx_signed_word(args[1]));return 0;
        default:break;
        }
    }
    if(segment==0x1f2a && offset==0xdb46) {
        assert(count==1);jx_npc_play_animation(c,(int16_t)jx_signed_word(args[0]));return 0;
    }
    if(segment==0x1f2a && offset==0xdba6) {
        assert(count==1);jx_npc_reverse_animation(c,(int16_t)jx_signed_word(args[0]));return 0;
    }
    return bridge->fallback(bridge->fallback_opaque,segment,offset,args,count);
}
void jx_engine_bridge_install(JxEventContext *c,JxEngineBridge *bridge) {
    assert(c && bridge && c->call && c->call!=dispatch);
    bridge->context=c;bridge->fallback=c->call;bridge->fallback_opaque=c->opaque;
    c->call=dispatch;c->opaque=bridge;
}
void jx_engine_bridge_remove(JxEventContext *c,JxEngineBridge *bridge) {
    assert(c->opaque==bridge && (c->call==dispatch || c->call==runtime_dispatch || c->call==graphics_memory_dispatch || c->call==inventory_dispatch));
    c->call=bridge->fallback;c->opaque=bridge->fallback_opaque;
}
void jx_runtime_bridge_install(JxEventContext *c,JxEngineBridge *bridge) {
    assert(c && bridge && c->call && c->call!=runtime_dispatch);
    bridge->context=c;bridge->fallback=c->call;bridge->fallback_opaque=c->opaque;
    c->call=runtime_dispatch;c->opaque=bridge;
}
void jx_graphics_memory_bridge_install(JxEventContext *c,JxEngineBridge *bridge) {
    assert(c && bridge && c->call && c->call!=graphics_memory_dispatch);
    bridge->context=c;bridge->fallback=c->call;bridge->fallback_opaque=c->opaque;
    c->call=graphics_memory_dispatch;c->opaque=bridge;
}
