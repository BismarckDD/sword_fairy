#include "event_engine.h"
/* Original 517F and529D banking helpers. Mode18 normally stays in bank0;
 * callbacks are retained for original low-word aperture wrap behavior. */
static void map_bank(JxEventContext *c,uint16_t bank,JxLegacyBankService service,void *opaque){
    jx_set_word(c,0x2d54,bank);uint16_t groups=jx_word(c,0x2d5a),remaining=groups;
    uint16_t base=jx_word(c,0x2d60);
    do{
        service(opaque,(JxFarPointer){jx_word(c,0x2cd4),jx_word(c,0x2cd6)},remaining,(uint16_t)(base+jx_word(c,0x2d54)));
        --remaining;base=jx_word(c,0x2d5e);
    }while(!remaining); /* Original51A1 is JE, not JNE. */
}
static void next_bank(JxEventContext *c,JxLegacyBankService service,void *opaque){map_bank(c,(uint16_t)(jx_word(c,0x2d58)+jx_word(c,0x2d54)),service,opaque);}
static uint16_t rol16(uint16_t value,unsigned shift){shift&=15;return (uint16_t)((value<<shift)|(value>>((16-shift)&15)));}
static uint16_t ror16(uint16_t value,unsigned shift){shift&=15;return (uint16_t)((value>>shift)|(value<<((16-shift)&15)));}
static uint32_t image_address(JxFarPointer p){return ((uint32_t)p.segment*16+p.offset)&0xfffff;}
static uint8_t image_read(JxEventContext *c,const JxVideoBus *bus,JxFarPointer p){uint32_t at=image_address(p);return at>=0xa0000&&at<0xb0000?bus->read_byte(bus->opaque,at):c->memory[at];}
static void image_write(JxEventContext *c,const JxVideoBus *bus,JxFarPointer p,uint8_t value){uint32_t at=image_address(p);if(at>=0xa0000&&at<0xb0000)bus->write_byte(bus->opaque,at,value);else c->memory[at]=value;}
static void advance_huge(JxFarPointer *p){++p->offset;if(!p->offset)p->segment=(uint16_t)(p->segment+0x1000);}
static void retreat_huge(JxFarPointer *p){if(!p->offset)p->segment=(uint16_t)(p->segment-0x1000);--p->offset;}
static void reg(const JxVideoBus *bus,uint16_t port,uint8_t index,uint8_t value){bus->out(bus->opaque,port,index,1);bus->out(bus->opaque,(uint16_t)(port+1),value,1);}
/* 1832:5D38. Keep the original look-ahead read on the final output byte. */
void jx_legacy_image_capture_scanline(JxEventContext *c,const JxVideoBus *bus,uint16_t plane,
    uint16_t bits,uint16_t shift,uint16_t last_mask,JxFarPointer *source,JxFarPointer *destination,
    JxLegacyBankService banking,void *opaque){
    reg(bus,0x3ce,4,(uint8_t)plane);
    uint8_t previous=image_read(c,bus,*source);++source->offset;if(!source->offset)next_bank(c,banking,opaque);
    for(;;){
        uint8_t following=image_read(c,bus,*source);++source->offset;
        uint16_t value=rol16((uint16_t)(((uint16_t)previous<<8)|following),shift);
        uint8_t output=(uint8_t)(value>>8);uint16_t before=bits;bits=(uint16_t)(bits-8);
        if(before<=8){image_write(c,bus,*destination,(uint8_t)(output&last_mask));advance_huge(destination);return;}
        if(!source->offset)next_bank(c,banking,opaque);
        image_write(c,bus,*destination,output);advance_huge(destination);previous=following;
    }
}
/* 1832:5D10. COPY/XOR/OR/AND/NOT operation setup from the resident table. */
void jx_legacy_image_prepare_write(JxEventContext *c,const JxVideoBus *bus,uint16_t operation){
    jx_ds(c)[0x2cdd]=0x88;jx_set_word(c,0x18e1,operation==2?0x5cf9:0x5cfb);
    uint8_t rop=jx_far_byte(c,(JxFarPointer){0x5d05,0x1832},(uint8_t)operation);reg(bus,0x3ce,3,rop);
}
/* 5CF9/5CFB. XCHG returns the actual read-plane byte in AH. */
static uint16_t put_byte(JxEventContext *c,const JxVideoBus *bus,uint16_t ax,JxFarPointer *destination,
    JxLegacyBankService banking,void *opaque){
    uint8_t data=(uint8_t)(ax>>8);if(jx_word(c,0x18e1)==0x5cf9)data=(uint8_t)~data;
    uint8_t old=image_read(c,bus,*destination);image_write(c,bus,*destination,data);
    ++destination->offset;if(!destination->offset)next_bank(c,banking,opaque);
    return (uint16_t)((ax&255)|((uint16_t)old<<8));
}
/* 1832:5D7D. Shift source bytes into the destination bit alignment, retaining
 * original partial-byte masks, latch reads and look-ahead pointer rewind. */
uint16_t jx_legacy_image_put_scanline(JxEventContext *c,const JxVideoBus *bus,uint16_t plane,uint16_t bits,
    uint16_t shift,uint16_t masks,JxFarPointer *source,JxFarPointer *destination,JxLegacyBankService banking,void *opaque){
    if(jx_word(c,0x18e1)!=0x5cf9&&jx_word(c,0x18e1)!=0x5cfb)return 0;
    /* Original5148 has no SS override: these two reads use the image's DS. */
    uint16_t source_bank=jx_far_word(c,source->segment,0x2cd0);
    if(source_bank!=jx_far_word(c,source->segment,0x2d54))map_bank(c,source_bank,banking,opaque);
    uint8_t rotate=jx_ds(c)[0x2cdd];rotate=(uint8_t)((rotate<<1)|(rotate>>7));jx_ds(c)[0x2cdd]=rotate;
    uint8_t start=(uint8_t)(masks>>8),end=(uint8_t)masks,selected=(uint8_t)(rotate&15);
    reg(bus,0x3ce,8,start);reg(bus,0x3ce,5,0);reg(bus,0x3ce,4,(uint8_t)plane);reg(bus,0x3c4,2,selected);
    uint16_t ax=(uint16_t)(((uint16_t)image_read(c,bus,*source)<<8)|selected);advance_huge(source);ax=ror16(ax,shift);
    uint16_t total=(uint16_t)(bits+shift),remaining=(uint16_t)(total-8);
    if(total>8){
        ax=put_byte(c,bus,ax,destination,banking,opaque);start=255;reg(bus,0x3ce,8,255);
        for(;;){
            ax=rol16(ax,shift);ax=(uint16_t)((ax&0xff00)|image_read(c,bus,*source));advance_huge(source);
            ax=(uint16_t)((ax<<8)|(ax>>8));ax=ror16(ax,shift);
            uint16_t before=remaining;remaining=(uint16_t)(remaining-8);
            if(before>8){ax=put_byte(c,bus,ax,destination,banking,opaque);continue;}
            remaining=(uint16_t)(remaining+8);if(shift>=remaining)retreat_huge(source);break;
        }
    }
    reg(bus,0x3ce,8,(uint8_t)(start&end));(void)put_byte(c,bus,ax,destination,banking,opaque);return 1;
}
/* 1832:6890. Derive starting shift and both edge masks from the pixel mask.
 * Return zero for an unsupported callback or a state that would never reach
 * the original SHL carry (zero mask/invalid bits-per-pixel alignment). */
uint16_t jx_legacy_image_setup(JxEventContext *c,const JxVideoBus *bus,JxFarPointer data,
    uint16_t bits,uint16_t operation,uint16_t put){
    jx_set_word(c,0x2da8,data.offset);jx_set_word(c,0x2daa,data.segment);jx_set_word(c,0x2da2,bits);
    uint8_t mask=jx_ds(c)[0x2cca],step=jx_ds(c)[0x18a4],start=0,shift=0,width=(uint8_t)bits;
    if(!mask||!step||step>8)return 0;
    for(;;){
        uint8_t carry=(uint8_t)((mask>>(8-step))&1);mask=(uint8_t)(mask<<step);
        if(carry)break;
        if(!mask)return 0;
        start|=mask;width=(uint8_t)(width+step);shift=(uint8_t)(shift+step);
    }
    uint8_t remainder=(uint8_t)((put?width:bits)&7);
    uint8_t end=remainder?(uint8_t)~(255u>>remainder):255;
    if(!put){jx_set_word(c,0x2da4,(uint16_t)(shift|((uint16_t)end<<8)));return 1;}
    jx_set_word(c,0x2da4,shift);jx_set_word(c,0x2da6,(uint16_t)(end|((uint16_t)(uint8_t)~start<<8)));
    if(jx_word(c,0x18d1)!=0x5d10)return 0;
    jx_legacy_image_prepare_write(c,bus,operation);return 1;
}
/* 1832:68EB. Each plane starts at the same aperture cursor; image data
 * appends plane by plane and normalizes segment carry through38AA. */
uint16_t jx_legacy_image_capture_row(JxEventContext *c,const JxVideoBus *bus,JxLegacyBankService banking,void *opaque){
    if(jx_word(c,0x18d3)!=0x5d38)return 0;
    JxFarPointer output={jx_word(c,0x2da8),jx_word(c,0x2daa)};
    unsigned planes=jx_ds(c)[0x18a5];if(!planes)planes=256;
    for(unsigned plane=0;plane<planes;++plane){
        JxFarPointer source={jx_word(c,0x2ccc),jx_word(c,0x2cce)};uint16_t masks=jx_word(c,0x2da4);
        jx_legacy_image_capture_scanline(c,bus,(uint16_t)plane,jx_word(c,0x2da2),(uint8_t)masks,masks>>8,&source,&output,banking,opaque);
    }
    jx_set_word(c,0x2daa,output.segment);jx_set_word(c,0x2da8,output.offset);return 1;
}
/* 1832:6924. Source image advances through the planes; aperture DI resets. */
uint16_t jx_legacy_image_put_row(JxEventContext *c,const JxVideoBus *bus,JxLegacyBankService banking,void *opaque){
    if(jx_word(c,0x18d5)!=0x5d7d)return 0;
    JxFarPointer source={jx_word(c,0x2da8),jx_word(c,0x2daa)};
    unsigned planes=jx_ds(c)[0x18a5];if(!planes)planes=256;
    for(unsigned plane=0;plane<planes;++plane){
        JxFarPointer destination={jx_word(c,0x2ccc),jx_word(c,0x2cce)};
        if(!jx_legacy_image_put_scanline(c,bus,(uint16_t)plane,jx_word(c,0x2da2),jx_word(c,0x2da4),jx_word(c,0x2da6),&source,&destination,banking,opaque))return 0;
    }
    jx_set_word(c,0x2daa,source.segment);jx_set_word(c,0x2da8,source.offset);return 1;
}
/* 1832:6DD4/6DBA. Comparisons are signed after low-word origin addition. */
uint16_t jx_legacy_image_contains(JxEventContext *c,uint16_t x,uint16_t y){
    return (int16_t)x>=(int16_t)jx_word(c,0x2db4)&&(int16_t)x<=(int16_t)jx_word(c,0x2db6)&&
        (int16_t)y>=(int16_t)jx_word(c,0x2db8)&&(int16_t)y<=(int16_t)jx_word(c,0x2dba);
}
uint16_t jx_legacy_image_translate_point(JxEventContext *c,uint16_t x,uint16_t y){
    x=(uint16_t)(x+jx_word(c,0x2dbc));y=(uint16_t)(y+jx_word(c,0x2dbe));
    jx_set_word(c,0x2e7c,x);jx_set_word(c,0x2e84,x);jx_set_word(c,0x2e7e,y);jx_set_word(c,0x2e86,y);
    return jx_legacy_image_contains(c,x,y);
}
/* Original0x3100/0x3127 and inline0x6F72 optional cursor/render callbacks remain explicit services.
 * Normal mode18 image calls have18E6>=0,2DF9=0 and1860=FFFF. */
uint16_t jx_legacy_image_begin(JxEventContext *c){
    if(jx_word(c,0x18ea)==0xd6d6)(void)JX_CALL0(c,0x1000,0x5020);
    uint16_t graphics=jx_ds(c)[0x187c]!=0;
    if(graphics){jx_set_word(c,0x1860,0xffff);if(jx_ds(c)[0x2df9]){(void)JX_CALL0(c,0x1832,0x3152);jx_ds(c)[0x2df9]=0;}}
    jx_ds(c)[0x2cb6]=0;return graphics;
}
void jx_legacy_image_end(JxEventContext *c){
    if(!jx_ds(c)[0x187c])return;
    if((int8_t)jx_ds(c)[0x18e6]<0&&!jx_ds(c)[0x2df9]){(void)JX_CALL0(c,0x1832,0x3152);++jx_ds(c)[0x2df9];}
    if(jx_word(c,0x1860)!=0xffff)(void)JX_CALL0(c,0x1832,0x5152);
}
static void image_word(JxEventContext *c,const JxVideoBus *bus,JxFarPointer p,uint16_t value){
    image_write(c,bus,p,(uint8_t)value);uint32_t next=(image_address(p)+1)&0xfffff;
    image_write(c,bus,(JxFarPointer){(uint16_t)(next&15),(uint16_t)(next>>4)},(uint8_t)(value>>8));
}
/* 1832:6DF7. Return1 after original success/error completion;0 indicates an
 * unsupported descriptor callback that must stop the host operation. */
int jx_legacy_image_capture(JxEventContext *c,const JxVideoBus *bus,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,
    JxFarPointer output,JxLegacyPointService point,JxLegacyNearService near_call,JxLegacyBankService banking,void *opaque){
    if(!jx_legacy_image_begin(c)){jx_ds(c)[0x2cb6]=0xfd;jx_legacy_image_end(c);return 1;}
    if(!jx_legacy_image_translate_point(c,x1,y1))goto outside;
    uint16_t first_x=jx_word(c,0x2e84),first_y=jx_word(c,0x2e86);
    if(!jx_legacy_image_translate_point(c,x2,y2))goto outside;
    uint16_t last_x=jx_word(c,0x2e84),last_y=jx_word(c,0x2e86);
    uint16_t left=first_x,right=last_x,top=first_y,bottom=last_y;
    if((int16_t)last_x<(int16_t)first_x){left=last_x;right=first_x;jx_ds(c)[0x2cb6]=3;}
    if((int16_t)last_y<(int16_t)first_y){top=last_y;bottom=first_y;jx_ds(c)[0x2cb6]=3;}
    jx_set_word(c,0x2e84,left);jx_set_word(c,0x2e86,top);
    uint16_t width=(uint16_t)(right-left+1),height=(uint16_t)(bottom-top+1);
    point(opaque,jx_word(c,0x18b3),left,top);
    uint16_t bits=(uint16_t)(width*jx_ds(c)[0x18a4]);image_word(c,bus,output,bits);
    JxFarPointer second=output;second.offset=(uint16_t)(second.offset+2);image_word(c,bus,second,height);
    output.offset=(uint16_t)(output.offset+4);
    if(!jx_legacy_image_setup(c,bus,output,bits,0,0))return 0;
    do{
        if(!jx_legacy_image_capture_row(c,bus,banking,opaque))return 0;
        near_call(opaque,jx_word(c,0x18bd),0);
    }while(--height);
    jx_legacy_image_end(c);return 1;
outside:
    jx_ds(c)[0x2cb6]=0xff;jx_legacy_image_end(c);return 1;
}
/* 1832:6E90. Original fully-outside rejection; this routine does not clip a
 * partially visible image. Return-1 for its original zero-divisor fault. */
int jx_legacy_image_put(JxEventContext *c,const JxVideoBus *bus,uint16_t x,uint16_t y,JxFarPointer image,uint16_t operation,
    JxLegacyPointService point,JxLegacyNearService near_call,JxLegacyBankService banking,void *opaque){
    uint8_t error;
    if(!jx_legacy_image_begin(c)){error=0xfd;goto failed;}
    if(operation>4){error=0xfc;goto failed;}
    if(!jx_legacy_image_translate_point(c,x,y)){error=0xff;goto failed;}
    x=jx_word(c,0x2e84);y=jx_word(c,0x2e86);point(opaque,jx_word(c,0x18b3),x,y);
    uint16_t bits=jx_far_word(c,image.segment,image.offset),height=jx_far_word(c,image.segment,(uint16_t)(image.offset+2));
    image.offset=(uint16_t)(image.offset+4);
    if((int16_t)bits<=0||bits>0x4000||(int16_t)height<=0){error=0xf7;goto failed;}
    uint8_t pixel_bits=jx_ds(c)[0x18a4];if(!pixel_bits)return -1;
    uint16_t end_x=(uint16_t)(x+bits/pixel_bits-1),end_y=(uint16_t)(y+height-1);
    if((int16_t)end_x<0||(int16_t)end_y<0||!jx_legacy_image_contains(c,end_x,end_y)){error=0xff;goto failed;}
    if(!jx_legacy_image_setup(c,bus,image,bits,operation,1))return 0;
    do{
        if(!jx_legacy_image_put_row(c,bus,banking,opaque))return 0;
        near_call(opaque,jx_word(c,0x18bd),0);
    }while(--height);
    near_call(opaque,jx_word(c,0x18c9),0);jx_legacy_image_end(c);return 1;
failed:
    jx_ds(c)[0x2cb6]=error;jx_legacy_image_end(c);return 1;
}
/* Successful 1832:69B3 mode18 path. Other requested modes and the BIOS
 * failure/restore branch are not supplied by this narrow entry point.
 * Return zero if the caller must stop for an unsupported path; result is
 * the original defined AL return on completed success/setup rejection. */
int jx_legacy_image_enter_mode18(JxEventContext *c,const JxLegacyVideoDevice *device,
    JxLegacyNearService near_call,JxLegacyPointService point,void *opaque,uint16_t *result){
    if(jx_ds(c)[0x2cc6]&0x80)return 0;
    uint16_t previous_graphics=jx_legacy_image_begin(c);jx_ds(c)[0x18e4]=0xff;
    if(!previous_graphics)jx_set_word(c,0x2dfa,(uint16_t)((jx_word(c,0x1880)&0xff00)|jx_ds(c)[0x187e]));
    if(jx_legacy_video_setup_mode18(c,near_call,opaque)){
        jx_ds(c)[0x2cb6]=0xfe;*result=0;jx_legacy_image_end(c);return 1;
    }
    uint16_t ax=jx_ds(c)[0x188e],selector=0;
    if(!jx_legacy_video_initialize_font_rows(c,device,ax,&ax))return 0;
    if(!jx_legacy_video_select_font(c,ax,&ax,&selector))return 0;
    jx_legacy_video_update_equipment(c,device);
    JxLegacyBiosRegisters registers={ax,0,selector,0,0,0,0,0};
    if(jx_word(c,0x1898)!=0x4949)return 0;
    jx_legacy_video_activate_planar(c,device,&registers);
    if(jx_legacy_video_check_mode(c,device))return 0;
    if(!jx_legacy_video_initialize_font_rows(c,device,0,&ax))return 0;
    jx_legacy_video_text_pages(c);
    if(jx_word(c,0x189c)!=0x4979)return 0;
    jx_legacy_video_initialize_palette(c,device);
    jx_set_word(c,0x2ded,0);jx_set_word(c,0x2deb,0);
    jx_set_word(c,0x2def,0);jx_set_word(c,0x2df1,0);
    if(jx_word(c,0x189a)!=0x4967)return 0;
    (void)jx_legacy_video_select_planar_page(c,0);
    if(!jx_ds(c)[0x187c])return 0;
    jx_legacy_video_reset_state(c,near_call,point,opaque);
    *result=(uint16_t)(jx_ds(c)[0x1881]&jx_ds(c)[0x18e4]);jx_legacy_image_end(c);return 1;
}
