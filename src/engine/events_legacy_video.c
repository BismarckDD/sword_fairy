#include "event_engine.h"
/* 1832:4EA2. Probe VESA and prefer mode101 to100, preserving the original
 * low-word mode-list cursor and early termination on FFFF. Private264-byte
 * controller-info storage belongs to the caller, outside live game state. */
uint16_t jx_legacy_video_probe_vesa(JxEventContext *c,const JxLegacyVideoDevice *device,JxFarPointer scratch){
    jx_set_word(c,0x2d62,0x13);
    JxLegacyBiosRegisters registers={0x4f00,0,0,0,0,scratch.offset,scratch.segment,0};
    device->bios(device->opaque,&registers);
    JxFarPointer result={scratch.offset,registers.es};
    if(registers.ax!=0x4f || jx_far_word(c,result.segment,result.offset)!=0x4556 ||
        jx_far_word(c,result.segment,(uint16_t)(result.offset+2))!=0x4153)return registers.ax;
    jx_ds(c)[0x2cc6]=0x88;jx_ds(c)[0x2cc7]=0x18;
    JxFarPointer list={jx_far_word(c,result.segment,(uint16_t)(result.offset+14)),jx_far_word(c,result.segment,(uint16_t)(result.offset+16))};
    for(;;){
        uint16_t mode=jx_far_word(c,list.segment,list.offset);list.offset=(uint16_t)(list.offset+2);
        if(mode<0x100)continue;
        if(mode==0xffff)return mode;
        if(mode>0x101)continue;
        jx_set_word(c,0x2d62,mode);if(mode==0x101)return mode;
    }
}
/* 1832:3661. Return the original carry bit; BIOS data reads are from the
 * supplied machine memory. Active monochrome/color display bits can remove
 * unsupported modes without replacing the discovered adapter category. */
uint16_t jx_legacy_video_check_display(JxEventContext *c){
    uint16_t feature=jx_far_word(c,0,0x488);
    if(feature&0x100)return 0;
    uint8_t flags=(uint8_t)feature;
    if(!(flags&8))flags^=2;
    uint8_t equipment=jx_far_byte(c,(JxFarPointer){0x410,0},0);
    jx_ds(c)[0x2cc3]=equipment;equipment&=0x30;
    if(equipment!=0x30)flags^=2;
    if(flags&2){
        if(equipment!=0x30){jx_set_word(c,0x2cbd,(uint16_t)(jx_word(c,0x2cbd)&0xfeff));jx_ds(c)[0x2cc7]&=0xf7;}
        else{jx_ds(c)[0x2cbf]=0;jx_set_word(c,0x2cbd,(uint16_t)(jx_word(c,0x2cbd)&0x100));jx_ds(c)[0x2cc7]&=0xef;}
        return 0;
    }
    jx_ds(c)[0x2cbf]=0;jx_set_word(c,0x2cbd,0);jx_ds(c)[0x2cc6]=2;jx_ds(c)[0x2cc7]=2;return 1;
}
/* 1832:6D06. All arithmetic wraps before the unsigned midpoint shift. */
void jx_legacy_video_viewport_center(JxEventContext *c){
    uint16_t left=jx_word(c,0x2db4),top=jx_word(c,0x2db8);
    uint16_t width=(uint16_t)(jx_word(c,0x2db6)-left);
    uint16_t height=(uint16_t)(jx_word(c,0x2dba)-top);
    jx_set_word(c,0x2dc0,width);jx_set_word(c,0x2dc2,height);
    jx_set_word(c,0x2e7c,(uint16_t)(left+((uint16_t)(width+1)>>1)));
    jx_set_word(c,0x2e7e,(uint16_t)(top+((uint16_t)(height+1)>>1)));
}
/* 1832:6AF3, including its near 50A7 mode13/VESA substitution. */
void jx_legacy_video_reset_modes(JxEventContext *c){
    uint8_t category=jx_ds(c)[0x2cc6],flags=jx_ds(c)[0x2cc7];
    uint8_t low=0,high=0;
    if(category!=1){
        low=0x13;high=0x12;
        if(!((category&8)&&(flags&0x10))){
            high=0x11;
            if(!(category&0x10)){
                low=8;
                if(category&0x20)high=low;
                else{
                    low=4;high=6;
                    if(!(category&2)){
                        low=15;
                        if(flags&9){high=low;goto modes_ready;}
                        else{
                            --low;
                            if(flags&2){high=low;goto modes_ready;}
                            else{high=0x10;if(jx_word(c,0x2cc8)>0x40)low=high;}
                        }
                    }
                    if(category&0x40)high=0x40;
                }
            }
        }
    }
modes_ready:;
    uint16_t bx=high;
    uint16_t ax=low==0x13?jx_word(c,0x2d62):low;
    if((uint8_t)ax==(uint8_t)(ax>>8))bx=ax;
    jx_set_word(c,0x2dac,ax);jx_set_word(c,0x2dae,bx);
}
/* Mode18 descriptor callbacks: VGA byte address and individual pixel mask. */
void jx_legacy_video_planar_address(JxEventContext *c,uint16_t x,uint16_t y){
    jx_set_word(c,0x2ccc,(uint16_t)(80u*y+(x>>3)));
    jx_ds(c)[0x2cca]=(uint8_t)(0x80u>>(x&7));
}
uint16_t jx_legacy_video_planar_left(JxEventContext *c){
    uint8_t mask=jx_ds(c)[0x2cca];uint16_t carry=mask>>7;
    jx_ds(c)[0x2cca]=(uint8_t)((mask<<1)|carry);
    if(carry)jx_set_word(c,0x2ccc,(uint16_t)(jx_word(c,0x2ccc)-1));
    return carry;
}
uint16_t jx_legacy_video_planar_up(JxEventContext *c,uint16_t checked){
    if(checked && jx_word(c,0x2ccc)<jx_word(c,0x2ce6))return 1;
    jx_set_word(c,0x2ccc,(uint16_t)(jx_word(c,0x2ccc)-jx_word(c,0x18a6)));return 0;
}
uint16_t jx_legacy_video_planar_down(JxEventContext *c,uint16_t checked){
    if(checked && jx_word(c,0x2ccc)>=jx_word(c,0x2cea))return 1;
    jx_set_word(c,0x2ccc,(uint16_t)(jx_word(c,0x2ccc)+jx_word(c,0x18a6)));return 0;
}
void jx_legacy_video_planar_color(JxEventContext *c,uint16_t color){
    uint8_t value=(uint8_t)color,maximum=jx_ds(c)[0x1888];
    jx_ds(c)[0x2ccb]=value>maximum?maximum:value;
}
static void legacy_register(const JxVideoBus *bus,uint16_t port,uint8_t index,uint8_t value){
    bus->out(bus->opaque,port,index,1);bus->out(bus->opaque,(uint16_t)(port+1),value,1);
}
/* 1832:38C5. Preserve individual byte OUT transactions and their order. */
void jx_legacy_video_reset_registers(JxEventContext *c,const JxVideoBus *bus){
    legacy_register(bus,0x3c4,2,15);
    legacy_register(bus,0x3ce,1,0);legacy_register(bus,0x3ce,2,0);
    legacy_register(bus,0x3ce,3,0);legacy_register(bus,0x3ce,8,255);
    legacy_register(bus,0x3ce,7,15);legacy_register(bus,0x3ce,5,(uint8_t)(jx_ds(c)[0x188f]&0x10));
}
/* 1832:5B74. Read four separate planes, collecting the selected bit. */
uint16_t jx_legacy_video_planar_read(JxEventContext *c,const JxVideoBus *bus){
    uint32_t address=((uint32_t)jx_word(c,0x2cce)*16+jx_word(c,0x2ccc))&0xfffff;
    legacy_register(bus,0x3ce,5,0);
    bus->out(bus->opaque,0x3ce,4,1);
    uint16_t color=0;
    for(int plane=3;plane>=0;--plane){
        bus->out(bus->opaque,0x3cf,(uint16_t)plane,1);
        uint8_t value=bus->read_byte(bus->opaque,address);
        color=(uint16_t)((color<<1)|((value&jx_ds(c)[0x2cca])!=0));
    }
    jx_legacy_video_reset_registers(c,bus);return color;
}
/* 1832:5BC4. XCHG loads VGA latches before the masked pixel write. */
void jx_legacy_video_planar_write(JxEventContext *c,const JxVideoBus *bus){
    legacy_register(bus,0x3ce,5,jx_ds(c)[0x188f]);
    legacy_register(bus,0x3ce,8,jx_ds(c)[0x2cca]);
    uint32_t address=((uint32_t)jx_word(c,0x2cce)*16+jx_word(c,0x2ccc))&0xfffff;
    (void)bus->read_byte(bus->opaque,address);
    bus->write_byte(bus->opaque,address,jx_ds(c)[0x2ccb]);
    bus->out(bus->opaque,0x3cf,255,1);
    legacy_register(bus,0x3ce,5,(uint8_t)(jx_ds(c)[0x188f]&0x10));
}
/* 1832:3A14. Text page offsets use the BIOS page size for non-25 rows. */
void jx_legacy_video_text_page_offsets(JxEventContext *c){
    if(jx_ds(c)[0x1881]!=25)jx_set_word(c,0x188b,(uint16_t)(jx_far_word(c,0,0x44c)>>4));
    uint16_t stride=(uint16_t)(jx_word(c,0x188b)<<4),offset=0;
    for(unsigned i=0;i<8;++i){jx_set_word(c,(uint16_t)(0x2cfe + 2*i),offset);offset=(uint16_t)(offset+stride);}
}
/* 1832:2F8E. Configure text-mode pages, leaving graphics modes untouched. */
void jx_legacy_video_text_pages(JxEventContext *c){
    if(jx_ds(c)[0x187c])return;
    uint8_t rows=jx_ds(c)[0x1881];
    if(rows!=25){
        uint8_t pages=(uint8_t)((rows&1)|6);
        if(jx_ds(c)[0x1880]!=40)pages>>=1;
        if((jx_ds(c)[0x2cc6]&4)&&jx_word(c,0x2cc8)<=64)pages>>=1;
        jx_ds(c)[0x188d]=pages;
    }
    jx_legacy_video_text_page_offsets(c);
}
/* 1832:2F34 / 4917. Return zero for the original byte-DIV overflow. */
static uint16_t legacy_select_rows(JxEventContext *c,uint16_t initial_ax,uint16_t *result,uint16_t *selector){
    uint8_t category=jx_ds(c)[0x2cc6],flags=jx_ds(c)[0x2cc7],rows=25;
    uint16_t ax=initial_ax;
    uint16_t font=jx_word(c,0x2cc6);
    if(category&0x1c){
        uint16_t mode=jx_word(c,0x187e);
        ax=mode;
        if(mode>16){
            if(!((category&0x40)&&mode==0x40)){
                uint16_t height=jx_word(c,0x1884);
                if(height/14>255)return 0;
                uint8_t requested=jx_ds(c)[0x1881],limit8=(uint8_t)(height>>3);
                uint8_t limit16=(uint8_t)(limit8>>1),quotient=(uint8_t)(height/14);
                if(requested>=limit8){rows=limit8;ax=(uint16_t)((uint16_t)requested<<8);font=1;}
                else if((category&0x10)||requested<quotient){rows=limit16;ax=(uint16_t)((uint16_t)limit8<<8);font=2;}
                else{rows=quotient;ax=(uint16_t)((uint16_t)requested<<8);font=0;}
            }
        }else{
            uint8_t supported=jx_ds(c)[0x184e + mode];
            ax=supported;
            if(!(category&8)){
                if(category&0x10)goto rows_done;
                supported&=5;
            }
            ax=0x184e;
            rows=jx_ds(c)[0x1881];if(rows==255)rows=50;
            if(rows==50){if(supported&8)goto rows_done;rows=43;}
            if(rows!=43 || !(supported&4) || (flags&2))rows=25;
        }
    }
rows_done:
    jx_ds(c)[0x1881]=rows;*result=(uint16_t)((ax&0xff00)|rows);if(selector)*selector=font;return 1;
}
uint16_t jx_legacy_video_select_rows(JxEventContext *c){uint16_t result;return legacy_select_rows(c,0,&result,0);}
uint16_t jx_legacy_video_select_font(JxEventContext *c,uint16_t ax,uint16_t *result,uint16_t *selector){return legacy_select_rows(c,ax,result,selector);}
/* 1832:37EA. Descriptor bytes are in the original resident code segment.
 * The caller supplies a near-call adapter; the descriptor selects the entry. */
void jx_legacy_video_load_descriptor(JxEventContext *c,uint16_t source,uint16_t length,
    JxLegacyNearService near_call,void *opaque){
    if(length==40)for(unsigned i=0;i<23;++i)jx_set_word(c,(uint16_t)(0x18b3+2*i),0x37e7);
    for(uint32_t i=0;i<length;++i)
        jx_ds(c)[(uint16_t)(0x187c+i)]=jx_far_byte(c,(JxFarPointer){source,0x1832},(uint16_t)i);
    near_call(opaque,jx_word(c,0x18bf),jx_ds(c)[0x188e]);
    jx_ds(c)[0x2cfc]=0;jx_set_word(c,0x2cd0,0);
    jx_set_word(c,0x2cd4,0x309d);jx_set_word(c,0x2cd6,0x1832);
}
/* 1832:6826. Original Olivetti ROM signature and adapter exclusions. */
uint16_t jx_legacy_video_probe_olivetti(JxEventContext *c){
    if(jx_far_word(c,0xfc00,0x50)!=0x4c4f || jx_far_word(c,0xfc00,0x52)!=0x5649)return 0;
    uint16_t model=jx_far_word(c,0xf000,0xfffd);
    if(model==0xff46 || model==0xfe00)return 0;
    if(jx_far_word(c,0xc000,0)!=0xaa55 || jx_far_word(c,0xc000,0x3c)==0x4150)return 0x40;
    if(jx_far_word(c,0xc000,0x10)!=0x4c4f)return 0;
    model=jx_far_word(c,0xc000,0x22);
    if(model==0x4756)return 0x40;
    if(model!=0x4745 || (jx_far_word(c,0,0x488)&0xa0)==0xa0)return 0;
    return 0x40;
}
/* 1832:36C1. Original carry return reports a failed mode switch.
 * Near1832:371A protects BP/SI/DI around INT10; C locals/ABI retain the
 * caller state while the explicit device callback supplies BIOS replies. */
uint16_t jx_legacy_video_check_mode(JxEventContext *c,const JxLegacyVideoDevice *device){
    if(jx_ds(c)[0x2cc6]&0x80)return jx_ds(c)[0x2cbc]!=0;
    JxLegacyBiosRegisters registers={0x0f00,0,0,0,0,0,0,0};
    device->bios(device->opaque,&registers);
    return (uint8_t)registers.ax!=jx_ds(c)[0x187e];
}
/* 1832:36DF. Match VGA equipment bits to the selected color/mono mode. */
void jx_legacy_video_update_equipment(JxEventContext *c,const JxLegacyVideoDevice *device){
    if(jx_ds(c)[0x2cc6]!=8)return;
    uint8_t equipment=jx_far_byte(c,(JxFarPointer){0x410,0},0)|0x30;
    uint16_t selector=0;
    if((jx_ds(c)[0x187e]&7)!=7){equipment&=0xef;selector=1;}
    jx_ds(c)[0x2cc3]=equipment;jx_far_set_byte(c,(JxFarPointer){0x410,0},0,equipment);
    if(!(jx_ds(c)[0x2cc4]&4)){
        JxLegacyBiosRegisters registers={(uint16_t)(0x1200|selector),0x33,0,0,0,0,0,0};
        device->bios(device->opaque,&registers);
    }
}
/* 1832:351B. BIOS and port replies belong to the device adapter; all adapter
 * category, mode masks, memory sizing and legacy fallbacks follow the code. */
void jx_legacy_video_detect_adapter(JxEventContext *c,const JxLegacyVideoDevice *device,
    const JxVideoBus *bus,JxFarPointer scratch){
    jx_ds(c)[0x2cc6]=2;jx_ds(c)[0x2cc7]=2;jx_ds(c)[0x2cc1]=0;jx_set_word(c,0x2cc8,16);
    uint16_t ax=jx_legacy_video_probe_vesa(c,device,scratch);
    JxLegacyBiosRegisters r={(uint16_t)(0x1b00|(ax&255)),0,0,0,0,0x2dfc,c->data_segment,0};
    device->bios(device->opaque,&r);
    uint8_t blocks;
    if((uint8_t)r.ax==0x1b){
        blocks=jx_ds(c)[0x2e2d];jx_ds(c)[0x2cc4]=jx_ds(c)[0x2e29];
        JxFarPointer modes={jx_word(c,0x2dfc),jx_word(c,0x2dfe)};
        jx_set_word(c,0x2cbd,jx_far_word(c,modes.segment,modes.offset));
        uint8_t extended=jx_far_byte(c,modes,2);jx_ds(c)[0x2cbf]=extended;
        if(!(jx_ds(c)[0x2cc6]&0x80)){
            jx_ds(c)[0x2cc6]=0x10;jx_ds(c)[0x2cc7]=0x10;
            if(!(extended&4))goto memory_size;
            jx_ds(c)[0x2cc6]=8;jx_ds(c)[0x2cc7]=0x18;
        }
        if(jx_legacy_video_check_display(c))goto fallback;
        jx_ds(c)[0x2cc1]=1;
    }else{
        r.ax=(uint16_t)(0x1200|(r.ax&255));r.bx=(uint16_t)((r.bx&0xff00)|0x10);
        device->bios(device->opaque,&r);blocks=(uint8_t)r.bx;
        if(blocks&0xfc)goto fallback;
        uint8_t mono=(jx_ds(c)[0x2cc3]&0x30)==0x30;
        if(mono!=(uint8_t)(r.bx>>8))goto fallback;
        jx_ds(c)[0x2cc6]=4;jx_ds(c)[0x2cc1]=1;
        uint8_t switches=(uint8_t)(r.cx&15);if(switches<=5)switches=(uint8_t)(switches+6);
        if(switches>=9)jx_ds(c)[0x2cc7]=switches==9?4:1;
    }
memory_size:
    jx_set_word(c,0x2cc8,(uint16_t)((uint8_t)(blocks+1)*64u));goto additional;
fallback:
    if((jx_ds(c)[0x2cc3]&0x30)==0x30){
        --jx_ds(c)[0x2cc7];--jx_ds(c)[0x2cc6];jx_set_word(c,0x2cc8,4);
    }
additional:
    if(jx_ds(c)[0x2cc6]&0x0e){
        uint16_t extra=jx_legacy_video_probe_olivetti(c);
        if(extra){jx_ds(c)[0x2cc6]|=(uint8_t)extra;if(jx_word(c,0x2cc8)<=32)jx_set_word(c,0x2cc8,32);}
    }
    r.ax=0xef00;r.dx=0xffff;device->bios(device->opaque,&r);
    if((uint8_t)r.dx!=255){
        if(jx_ds(c)[0x2cc6]&1){
            jx_ds(c)[0x2cc6]=0x20;jx_set_word(c,0x2cc8,64);
            if(r.dx>>8)goto retrace;
            jx_set_word(c,0x2cc8,32);
        }
        bus->out(bus->opaque,0x3bf,1,1);
    }
retrace:
    if(jx_ds(c)[0x2cc6]==2){
        jx_ds(c)[0x2cc5]=1;uint8_t first=(uint8_t)(bus->in_byte(bus->opaque,0x3da)&1);
        for(uint32_t i=0;i<65536;++i)if((bus->in_byte(bus->opaque,0x3da)&1)!=first)return;
        jx_ds(c)[0x2cc5]=0;
    }
}
/* Mode0/1/2/3/7 helpers sharing 1832:3992; return original carry. */
uint16_t jx_legacy_video_setup_text_mode(JxEventContext *c,uint16_t mode,JxLegacyNearService near_call,void *opaque){
    uint16_t source,attribute=0;
    if(mode<=3){
        if(!(jx_ds(c)[0x2cc7]&0x16))return 1;
        source=mode<=1?0x390a:0x3932;attribute=mode&1;
    }else if(mode==7){if(!(jx_ds(c)[0x2cc7]&9))return 1;source=0x395a;}
    else return 1;
    jx_legacy_video_load_descriptor(c,source,40,near_call,opaque);
    jx_ds(c)[0x187d]=(uint8_t)attribute;jx_set_word(c,0x187e,mode);
    if(!(jx_ds(c)[0x2cc6]&0x23)){
        uint8_t pages=7;
        if((jx_ds(c)[0x2cc6]&4)&&jx_word(c,0x2cc8)<=64&&mode>1)pages>>=1;
        jx_ds(c)[0x188d]=pages;if(jx_ds(c)[0x2cc7]&2)jx_ds(c)[0x1889]=15;
    }
    if(mode!=7&&(jx_ds(c)[0x2cc7]&0x1c))jx_set_word(c,0x1890,0x646e);
    return 0;
}
/* 1832:2F01. Result preserves original AX, including configured-row paths.
 * Return zero when its 2F34 helper would trigger the original DIV fault. */
uint16_t jx_legacy_video_initialize_font_rows(JxEventContext *c,const JxLegacyVideoDevice *device,
    uint16_t initial_ax,uint16_t *result){
    uint8_t saved=jx_ds(c)[0x185f];jx_ds(c)[0x185f]=0;
    if(saved){
        jx_ds(c)[0x1881]=saved;
        if(!legacy_select_rows(c,initial_ax,result,0))return 0;
        if(saved==255)saved=(uint8_t)*result;
        if(saved==(uint8_t)*result){jx_ds(c)[0x1881]=saved;return 1;}
    }
    JxLegacyBiosRegisters r={0x1130,0,0,24,0,0,0,0};device->bios(device->opaque,&r);
    uint8_t rows=(uint8_t)(r.dx+1);jx_ds(c)[0x1881]=rows;
    *result=(uint16_t)((uint8_t)r.cx*rows);return 1;
}
/* 1832:3448. Rounded scaling uses the original byte DIV, including faults. */
uint16_t jx_legacy_video_scale_cursor(JxEventContext *c,uint16_t shape,uint16_t numerator,uint16_t denominator,uint16_t *result){
    uint8_t multiplier=(uint8_t)numerator,divisor=(uint8_t)denominator;
    if(jx_ds(c)[0x1881]==25 && jx_word(c,0x1884)!=200 &&
        (jx_word(c,0x187e)==7 || !(jx_ds(c)[0x2cc6]&0x0c) || (jx_far_byte(c,(JxFarPointer){0x487,0},0)&1))){
        uint16_t bottom=(uint16_t)((shape&31)*multiplier+(divisor>>1));
        uint16_t top=(uint16_t)(((shape>>8)&31)*multiplier+(divisor>>1));
        if(!divisor || bottom/divisor>255 || top/divisor>255)return 0;
        shape=(uint16_t)((bottom/divisor)|(((top/divisor)|((shape>>8)&0x20))<<8));
    }
    *result=(uint16_t)(shape&0x3f1f);return 1;
}
/* 1832:3414: canonical seven-line cursor stored in the mode descriptor. */
uint16_t jx_legacy_video_set_cursor_shape(JxEventContext *c,uint16_t shape,uint16_t *result){
    if(!jx_legacy_video_scale_cursor(c,shape,100,(uint8_t)(jx_word(c,0x1884)>>1),result))return 0;
    uint8_t bottom=(uint8_t)*result,top=(uint8_t)((*result>>8)&31),hidden=(uint8_t)((*result>>8)&0x20);
    if(bottom>=7)bottom=7;if(top>=7)top=7;
    *result=(uint16_t)(bottom|((uint16_t)(top|hidden)<<8));jx_set_word(c,0x1878,*result);return 1;
}
/* 1832:3439. Shapes with unsupported bits bypass inverse scaling. */
uint16_t jx_legacy_video_unscale_cursor(JxEventContext *c,uint16_t shape,uint16_t *result){
    if(shape&0xd8f8){*result=(uint16_t)(shape&0x3f1f);return 1;}
    return jx_legacy_video_scale_cursor(c,shape,(uint8_t)(jx_word(c,0x1884)>>1),100,result);
}
/* 1832:34B8 with its 375B cursor query. The BIOS wrapper preserves DI/SI/BP;
 * only documented output fields enter the captured mode state. */
static void legacy_capture_state(JxEventContext *c,const JxLegacyVideoDevice *device,
    const JxVideoBus *bus,JxFarPointer scratch,JxLegacyBiosRegisters *result){
    uint8_t equipment=jx_far_byte(c,(JxFarPointer){0x410,0},0);
    jx_set_word(c,0x2cc2,(uint16_t)(equipment|((uint16_t)equipment<<8)));
    JxLegacyBiosRegisters r={(uint16_t)(0x0f00|equipment),0,0,0,0,0,0,0};
    device->bios(device->opaque,&r);
    uint8_t page=(uint8_t)(r.bx>>8),columns=(uint8_t)(r.ax>>8);
    uint16_t pages=(uint16_t)(page|((uint16_t)page<<8));
    jx_ds(c)[0x1874]=(uint8_t)r.ax;jx_ds(c)[0x187e]=(uint8_t)r.ax;jx_set_word(c,0x187a,pages);
    jx_legacy_video_detect_adapter(c,device,bus,scratch);
    r.ax=0x1130;r.bx=0;r.cx=0;r.dx=24;device->bios(device->opaque,&r);++r.dx;
    if(jx_ds(c)[0x2cc6]&0x80){
        r.ax=0x4f03;device->bios(device->opaque,&r);
        if(r.ax==0x4f)jx_set_word(c,0x187e,r.bx);
    }
    uint16_t dimensions=(uint16_t)(columns|((uint16_t)(uint8_t)r.dx<<8));
    jx_set_word(c,0x1880,dimensions);jx_set_word(c,0x1876,dimensions);
    uint16_t saved_bx=r.bx;
    r.ax=(uint16_t)(0x0300|columns);r.bx=(uint16_t)((r.bx&255)|((uint16_t)page<<8));r.dx=pages;
    device->bios(device->opaque,&r);
    r.dx=(uint16_t)(r.dx+1);r.dx=(uint16_t)((r.dx<<8)|(r.dx>>8));r.dx=(uint16_t)(r.dx+1);
    if((jx_ds(c)[0x2cc6]&0x0c)&&!jx_ds(c)[0x1318]&&(jx_far_byte(c,(JxFarPointer){0x487,0},0)&1)){
        uint16_t port=jx_far_word(c,0,0x463);
        bus->out(bus->opaque,port,10,1);uint8_t start=bus->in_byte(bus->opaque,(uint16_t)(port+1));
        bus->out(bus->opaque,port,11,1);uint8_t end=bus->in_byte(bus->opaque,(uint16_t)(port+1));
        r.cx=(uint16_t)(end|((uint16_t)start<<8));
    }
    jx_set_word(c,0x1872,r.cx);jx_set_word(c,0x1870,r.cx);
    r.ax=dimensions;r.bx=saved_bx;*result=r;
}
void jx_legacy_video_capture_state(JxEventContext *c,const JxLegacyVideoDevice *device,
    const JxVideoBus *bus,JxFarPointer scratch){JxLegacyBiosRegisters result;legacy_capture_state(c,device,bus,scratch,&result);}
/* 1832:32F7. Compose a text attribute or apply a planar color callback. */
void jx_legacy_video_compose_color(JxEventContext *c,JxLegacyNearService near_call,void *opaque){
    uint8_t value=jx_ds(c)[0x2dcc];
    if(!jx_ds(c)[0x187c])value=(uint8_t)((value&15)|((value&0x10)<<3)|((jx_ds(c)[0x2dc8]<<4)&0x70));
    else if(jx_ds(c)[0x18a5]==2){near_call(opaque,jx_word(c,0x18bf),value);value=jx_ds(c)[0x2ccb];}
    jx_ds(c)[0x2dcd]=value;
}
/* 1832:32CE. Return the old foreground color after validating the new one. */
uint16_t jx_legacy_video_set_foreground(JxEventContext *c,uint16_t color,JxLegacyNearService near_call,void *opaque){
    uint8_t maximum=jx_ds(c)[0x1888],limit=jx_ds(c)[0x187c]?maximum:31;
    if((color>>8)||(uint8_t)color>limit){color=maximum;jx_ds(c)[0x2cb6]=3;}
    uint8_t old=jx_ds(c)[0x2dcc];jx_ds(c)[0x2dcc]=(uint8_t)color;
    jx_legacy_video_compose_color(c,near_call,opaque);return old;
}
/* Original 2FBE text-mode branch. Graphics viewport setup is separate. */
void jx_legacy_video_reset_text_state(JxEventContext *c,JxLegacyNearService near_call,void *opaque){
    jx_set_word(c,0x2dc8,0);jx_set_word(c,0x2dca,0);
    (void)jx_legacy_video_set_foreground(c,jx_ds(c)[0x188e],near_call,opaque);
    jx_set_word(c,0x2df1,0);jx_set_word(c,0x2def,0);
    jx_ds(c)[0x2df7]=0;jx_ds(c)[0x2df9]=0;jx_ds(c)[0x2dd8]=0;jx_ds(c)[0x2cb7]=0;jx_ds(c)[0x2df8]=1;
    uint16_t ax=(uint16_t)((uint16_t)jx_ds(c)[0x1880]-1);jx_set_word(c,0x2df5,ax);
    ax=(uint16_t)((ax&0xff00)|jx_ds(c)[0x1881]);jx_set_word(c,0x2df3,(uint16_t)(ax-1));
}
/* Text descriptors install 37E7 STC;RET. The constructor does not consume
 * its carry result; this callback interface transports memory effects only. */
static void legacy_text_noop(void *opaque,uint16_t entry,uint16_t ax){(void)opaque;(void)entry;(void)ax;}
/* Original 3020 constructor for a DOS text-mode boot. Return zero on an
 * unsupported initial graphics mode, unavailable text descriptor or DIV fault.
 * That status must stop host initialization; it is not a replacement fallback. */
uint16_t jx_legacy_video_initialize_text(JxEventContext *c,const JxLegacyVideoDevice *device,
    const JxVideoBus *bus,JxFarPointer scratch){
    if(jx_ds(c)[0x187c])return 0;
    jx_set_word(c,0x2e90,0xffff);jx_ds(c)[0x2ea8]=0;jx_set_word(c,0x2ea6,0);
    if(!jx_ds(c)[0x1318])jx_ds(c)[0x2cb8]=(uint8_t)(jx_far_byte(c,(JxFarPointer){0x487,0},0)&1);
    JxLegacyBiosRegisters r;legacy_capture_state(c,device,bus,scratch,&r);
    r.ax=(uint16_t)(0x0300|(r.ax&255));
    device->bios(device->opaque,&r);
    jx_set_word(c,0x2ded,(uint8_t)r.dx);jx_set_word(c,0x2deb,(uint8_t)(r.dx>>8));
    uint16_t mode=jx_word(c,0x187e);
    if(jx_legacy_video_setup_text_mode(c,mode,legacy_text_noop,0))return 0;
    uint16_t ax=(uint16_t)(mode|((jx_ds(c)[0x2cc6]&0x23)?0:((uint16_t)jx_ds(c)[0x188d]<<8)));
    if(!jx_legacy_video_initialize_font_rows(c,device,ax,&ax))return 0;
    if(ax)jx_set_word(c,0x1884,ax);
    if(!jx_legacy_video_set_cursor_shape(c,jx_word(c,0x1870),&ax))return 0;
    jx_legacy_video_text_pages(c);
    r.ax=(uint16_t)(0x0500|(jx_word(c,0x187a)>>8));device->bios(device->opaque,&r);
    jx_legacy_video_reset_modes(c);jx_legacy_video_reset_text_state(c,legacy_text_noop,0);return 1;
}
/* 1832:3832. Build four clipping addresses while preserving the current
 * aperture cursor. The descriptor callback consumes CX=x,DX=y. */
void jx_legacy_video_clip_addresses(JxEventContext *c,uint16_t left,uint16_t right,uint16_t top,uint16_t bottom,
    JxLegacyPointService point,void *opaque){
    uint16_t saved_offset=jx_word(c,0x2ccc),saved_high=jx_word(c,0x2cd0);uint8_t saved_mask=jx_ds(c)[0x2cca];
    point(opaque,jx_word(c,0x18b3),0,bottom);
    jx_set_word(c,0x2cea,jx_word(c,0x2ccc));jx_set_word(c,0x2cec,jx_word(c,0x2cd0));
    point(opaque,jx_word(c,0x18b3),0,top);
    uint32_t sum=(uint32_t)jx_word(c,0x2ccc)+jx_word(c,0x18a6);
    jx_set_word(c,0x2ce6,(uint16_t)sum);jx_set_word(c,0x2ce8,(uint16_t)(jx_word(c,0x2cd0)+(sum>>16)));
    point(opaque,jx_word(c,0x18b3),left,0);
    jx_set_word(c,0x2cf6,jx_word(c,0x2ccc));jx_ds(c)[0x2cfa]=jx_ds(c)[0x2cca];
    point(opaque,jx_word(c,0x18b3),right,0);
    jx_set_word(c,0x2cf8,jx_word(c,0x2ccc));jx_ds(c)[0x2cfb]=jx_ds(c)[0x2cca];
    jx_ds(c)[0x2cca]=saved_mask;jx_set_word(c,0x2ccc,saved_offset);jx_set_word(c,0x2cd0,saved_high);
}
/* 1832:6D51. Inline0x372E graphics condition and0x3723 dimensions-minus-one
 * retain their caller-visible effects. Original1000:5011 calls empty5010.
 * Standalone CPU flags/register ABI of the near helpers is not exposed. */
void jx_legacy_video_reset_viewport(JxEventContext *c,JxLegacyPointService point,void *opaque){
    jx_ds(c)[0x2ea8]=0;if(!jx_ds(c)[0x187c])return;
    uint16_t right=(uint16_t)(jx_word(c,0x1882)-1),bottom=(uint16_t)(jx_word(c,0x1884)-1);
    jx_set_word(c,0x2db0,right);jx_set_word(c,0x2db2,bottom);
    jx_set_word(c,0x2db4,0);jx_set_word(c,0x2db8,0);jx_set_word(c,0x2dbc,0);jx_set_word(c,0x2dbe,0);
    jx_set_word(c,0x2db6,right);jx_set_word(c,0x2dba,bottom);
    jx_legacy_video_clip_addresses(c,0,right,0,bottom,point,opaque);jx_legacy_video_viewport_center(c);
    point(opaque,jx_word(c,0x18b3),jx_word(c,0x2e7c),jx_word(c,0x2e7e));
}
/* 1832:6D35 (6DA6 reaches this after clearing2EA6). */
void jx_legacy_video_reset_view(JxEventContext *c,JxLegacyPointService point,void *opaque){
    jx_set_word(c,0x2e90,0xffff);jx_legacy_video_reset_viewport(c,point,opaque);jx_set_word(c,0x2ea6,0);
}
/* 1832:2FBE. Class1 invokes its original3331 palette helper through the
 * caller's near adapter. Mode18 uses class2 and needs no palette helper. */
void jx_legacy_video_reset_state(JxEventContext *c,JxLegacyNearService near_call,JxLegacyPointService point,void *opaque){
    if(jx_ds(c)[0x187c]){
        jx_legacy_video_reset_view(c,point,opaque);
        jx_set_word(c,0x2dc4,jx_word(c,0x2e7c));jx_set_word(c,0x2dc6,jx_word(c,0x2e7e));
        jx_set_word(c,0x2dce,0xffff);jx_ds(c)[0x2cc0]=3;
        if(jx_ds(c)[0x187c]==1)near_call(opaque,0x3331,3);
    }
    jx_legacy_video_reset_text_state(c,near_call,opaque);
}
/* 1832:654B. Read the existing DAC first: FFFF:FFFF entries are preserved. */
void jx_legacy_video_palette_block(JxEventContext *c,const JxLegacyVideoDevice *device,JxFarPointer colors,JxFarPointer scratch){
    uint16_t count=(uint16_t)(jx_ds(c)[0x1888]+1);
    JxLegacyBiosRegisters r={0x1017,0,count,scratch.offset,colors.offset,scratch.offset,scratch.segment,0};
    device->bios(device->opaque,&r);
    JxFarPointer result={scratch.offset,r.es};
    for(uint16_t i=0;i<count;++i){
        uint16_t offset=(uint16_t)(colors.offset+4*i);
        uint16_t first=jx_far_word(c,colors.segment,offset),second=jx_far_word(c,colors.segment,(uint16_t)(offset+2));
        if(first==0xffff && second==0xffff)continue;
        jx_far_set_byte(c,result,(uint16_t)(3*i),(uint8_t)(first&63));
        jx_far_set_byte(c,result,(uint16_t)(3*i+1),(uint8_t)((first>>8)&63));
        jx_far_set_byte(c,result,(uint16_t)(3*i+2),(uint8_t)(second&63));
    }
    r.ax=0x1012;r.cx=count;r.dx=scratch.offset;device->bios(device->opaque,&r);
}
/* 1832:6CA3. Success returns FFFF, failure zero; source zero sets error FC. */
uint16_t jx_legacy_video_set_palette(JxEventContext *c,JxFarPointer source,JxLegacyPaletteService palette,void *opaque){
    if(jx_word(c,0x18ea)==0xd6d6)(void)JX_CALL0(c,0x1000,0x5020);
    if(!(source.offset|source.segment)){jx_ds(c)[0x2cb6]=0xfc;return 0;}
    uint16_t carry=palette(opaque,jx_word(c,0x18a2),source);
    uint16_t success=carry?0:0xffff;
    if(success && jx_ds(c)[0x187c]){
        jx_set_word(c,0x2dc8,jx_far_word(c,source.segment,source.offset));
        jx_set_word(c,0x2dca,jx_far_word(c,source.segment,(uint16_t)(source.offset+2)));
    }
    jx_ds(c)[0x2cb6]=(uint8_t)~success;return success;
}
static void legacy_bios_preserved(const JxLegacyVideoDevice *device,JxLegacyBiosRegisters *r){
    uint16_t si=r->si,di=r->di;device->bios(device->opaque,r);r->si=si;r->di=di;
}
/* 1832:4949. Font selector CX is computed by2F34 before mode activation. */
void jx_legacy_video_activate_planar(JxEventContext *c,const JxLegacyVideoDevice *device,JxLegacyBiosRegisters *r){
    r->ax=(uint8_t)jx_word(c,0x187e);legacy_bios_preserved(device,r);
    r->dx=(uint16_t)((r->dx&0xff00)|jx_ds(c)[0x1881]);
    uint16_t mode_result=r->ax;r->ax=(uint16_t)(r->cx+0x1122);r->cx=mode_result;r->bx=0;
    jx_set_word(c,0x2cf0,0);jx_set_word(c,0x2cd0,0);legacy_bios_preserved(device,r);
}
/* 1832:4967. Mode18 pages share the descriptor's aperture segment. */
uint16_t jx_legacy_video_select_planar_page(JxEventContext *c,uint16_t pages){
    jx_set_word(c,0x187a,pages);uint16_t segment=jx_word(c,0x1886);jx_set_word(c,0x2cce,segment);return segment;
}
/* 1832:4979. Default attribute table and sixteen DAC entries. */
void jx_legacy_video_initialize_palette(JxEventContext *c,const JxLegacyVideoDevice *device){
    JxLegacyBiosRegisters r={0,0,0,0,0,0,0x1832,0};
    if(jx_ds(c)[0x2cc1]){
        r.ax=0x1010;r.bx=16;legacy_bios_preserved(device,&r);
        r.dx=(jx_word(c,0x187e)>=17 && jx_ds(c)[0x1888]==1)?0x4577:0x4588;
        r.ax=0x1002;legacy_bios_preserved(device,&r);
    }
    r.ax=0x1012;r.bx=0;r.cx=16;r.dx=jx_word(c,0x1894);r.es=jx_word(c,0x1896);
    legacy_bios_preserved(device,&r);
}
/* 1832:48BE. Return the original carry after installing the mode18 table. */
uint16_t jx_legacy_video_setup_mode18(JxEventContext *c,JxLegacyNearService near_call,void *opaque){
    if(!(jx_ds(c)[0x2cc7]&0x10)||!(jx_ds(c)[0x2cbf]&4))return 1;
    jx_legacy_video_load_descriptor(c,0x44ad,101,near_call,opaque);return 0;
}




