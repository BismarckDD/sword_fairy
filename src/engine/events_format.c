#include "event_engine.h"
/* 1000:2A8C. Preserve shared string-stream state and post-format effects;
 * do not substitute host sprintf (different 16-bit ABI and side effects). */
static uint16_t sprintf_impl(JxEventContext *c,JxFarPointer destination,
    JxFarPointer format,JxFarPointer arguments,const JxFarPointer *workspace) {
    jx_ds(c)[0x2b88]=0x42;
    jx_set_word(c,0x2b84,destination.offset);
    jx_set_word(c,0x2b86,destination.segment);
    jx_set_word(c,0x2b7e,destination.offset);
    jx_set_word(c,0x2b80,destination.segment);
    jx_set_word(c,0x2b82,0x7fff);
    uint16_t result=workspace?jx_format_words(c,(JxFarPointer){0x2b7e,c->data_segment},
        format,arguments,*workspace):(uint16_t)JX_CALL(c,0x1000,0x19b6,
        0x2b7e,c->data_segment,format.offset,format.segment,
        arguments.offset,arguments.segment);
    uint16_t remaining=(uint16_t)(jx_word(c,0x2b82)-1);
    jx_set_word(c,0x2b82,remaining);
    if(jx_signed_word(remaining)>=0) {
        uint16_t offset=jx_word(c,0x2b7e),segment=jx_word(c,0x2b80);
        jx_set_word(c,0x2b7e,(uint16_t)(offset+1));
        c->memory[((uint32_t)segment*16+offset)&0xfffff]=0;
    } else {
        (void)JX_CALL(c,0x1000,0x0b76,0,0x2b7e,c->data_segment);
    }
    return result;
}

/* Word access crosses the physical next byte at offset FFFF, matching x86. */
static void format_word(JxEventContext *c,JxFarPointer p,uint16_t displacement,uint16_t value) {
    uint32_t at=((uint32_t)p.segment*16+(uint16_t)(p.offset+displacement))&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
}
static uint16_t format_read(JxEventContext *c,JxFarPointer p,uint16_t displacement) {
    return jx_far_word(c,p.segment,(uint16_t)(p.offset+displacement));
}
/* 1000:1DFE: only AX=FFFF is failure; preserve signed-byte argument to flsbuf. */
uint16_t jx_format_put_byte(JxEventContext *c,JxFarPointer stream,uint8_t value) {
    uint16_t remaining=(uint16_t)(format_read(c,stream,4)-1);
    format_word(c,stream,4,remaining);
    if(jx_signed_word(remaining)>=0) {
        uint16_t offset=format_read(c,stream,0);
        format_word(c,stream,0,(uint16_t)(offset+1));
        uint16_t segment=format_read(c,stream,2);
        jx_far_set_byte(c,(JxFarPointer){offset,segment},0,value);
        return 0;
    }
    uint16_t result=(uint16_t)JX_CALL(c,0x1000,0x0b76,
        (uint16_t)jx_signed_byte(value),stream.offset,stream.segment);
    return result==0xffff ? 0xffff : 0;
}
/* 1000:1E4D and 1E2F: finish the entire block after an output error. */
uint16_t jx_format_repeat(JxEventContext *c,JxFarPointer stream,uint8_t value,
    uint16_t count,uint16_t total) {
    uint16_t error=0;total=(uint16_t)(total+count);
    for(unsigned i=0;i<count;i++) error|=jx_format_put_byte(c,stream,value);
    return error ? 0xffff : total;
}
uint16_t jx_format_span(JxEventContext *c,JxFarPointer stream,JxFarPointer source,
    uint16_t count,uint16_t total) {
    uint16_t error=0;total=(uint16_t)(total+count);
    for(unsigned i=0;i<count;i++)
        error|=jx_format_put_byte(c,stream,jx_far_byte(c,source,(uint16_t)i));
    return error ? 0xffff : total;
}
/* 1000:1E69: unsigned 32-bit division, backward output, signed minimum width.
 * Zero with precision zero emits nothing. Hex letter adjustment is 7 or 39. */
JxFormatDigits jx_format_digits(JxEventContext *c,JxFarPointer end,uint32_t value,
    uint16_t radix,uint16_t minimum,uint8_t letter_adjustment) {
    uint16_t offset=end.offset;
    while(jx_signed_word(minimum)>0 || value) {
        uint8_t digit=(uint8_t)(value%radix+'0');value/=radix;
        if(digit>'9') digit=(uint8_t)(digit+letter_adjustment);
        jx_far_set_byte(c,(JxFarPointer){offset,end.segment},0,digit);
        offset=(uint16_t)(offset-1);minimum=(uint16_t)(minimum-1);
    }
    return (JxFormatDigits){{(uint16_t)(offset+1),end.segment},(uint16_t)(end.offset-offset)};
}

static JxFarPointer format_at(JxFarPointer p,uint16_t n) {
    p.offset=(uint16_t)(p.offset+n);return p;
}
static uint16_t format_argument(JxEventContext *c,JxFarPointer *p) {
    uint16_t value=format_read(c,*p,0);p->offset=(uint16_t)(p->offset+2);return value;
}
static uint32_t format_long(JxEventContext *c,JxFarPointer *p) {
    uint32_t low=format_argument(c,p);return low|((uint32_t)format_argument(c,p)<<16);
}
static JxFarPointer format_pointer(JxEventContext *c,JxFarPointer *p,uint16_t flags) {
    uint16_t offset=format_argument(c,p);
    uint16_t segment=(flags&0x20)?format_argument(c,p):(offset?c->data_segment:0);
    return (JxFarPointer){offset,segment};
}
static uint16_t format_length(JxEventContext *c,JxFarPointer p,uint16_t limit) {
    uint16_t length=0;
    while(length<limit && jx_far_byte(c,p,length)) length++;
    return length;
}
static void format_float_call(JxEventContext *c,uint16_t table,JxFarPointer text) {
    (void)JX_CALL(c,jx_word(c,(uint16_t)(table+2)),jx_word(c,table),text.offset,text.segment);
}
/* Typed output path keeps abnormal stack flow separate from FFFF I/O error. */
static JxFileWriteResult format_put_dos(JxEventContext *c,JxFarPointer stream,uint8_t value,
    JxFarPointer character,const JxCrtServices *services) {
    uint16_t remaining=(uint16_t)(format_read(c,stream,4)-1);
    format_word(c,stream,4,remaining);
    if(jx_signed_word(remaining)>=0) {
        uint16_t offset=format_read(c,stream,0);format_word(c,stream,0,(uint16_t)(offset+1));
        jx_far_set_byte(c,(JxFarPointer){offset,format_read(c,stream,2)},0,value);
        return (JxFileWriteResult){0,0};
    }
    format_word(c,character,0,(uint16_t)jx_signed_byte(value));
    JxFileWriteResult result=jx_stream_overflow_dos(c,stream.offset,character,services);
    if(!result.flow) result.value=result.value==65535?65535:0;
    return result;
}
static JxFileWriteResult format_output(JxEventContext *c,JxFarPointer stream,
    JxFarPointer source,uint8_t repeated,int repeat,uint16_t count,uint16_t total,
    JxFarPointer character,const JxCrtServices *services) {
    if(!services) return (JxFileWriteResult){0,repeat?jx_format_repeat(c,stream,repeated,count,total):jx_format_span(c,stream,source,count,total)};
    uint16_t error=0;total=(uint16_t)(total+count);
    for(unsigned i=0;i<count;i++) {
        uint8_t value=repeat?repeated:jx_far_byte(c,source,(uint16_t)i);
        JxFileWriteResult result=format_put_dos(c,stream,value,character,services);
        if(result.flow) return result;
        error|=result.value;
    }
    return (JxFileWriteResult){0,error?65535:total};
}
/* 1000:19B6, table-driven format parser. State table remains live in DS.
 * Host locals replace compiler BP locals; workspace retains far buffer ABI. */
static JxFileWriteResult format_words(JxEventContext *c,JxFarPointer stream,JxFarPointer format,
    JxFarPointer arguments,JxFarPointer workspace,const JxCrtServices *services) {
    uint16_t total=0,flags=0,width=0,precision=0,prefix_count=0,uppercase=0;
    uint8_t state=0;
    JxFarPointer prefix=format_at(workspace,518),character=format_at(workspace,530);
    JxFileWriteResult emitted;
    for(;;) {
        uint8_t ch=jx_far_byte(c,format,0);format.offset++;
        if(!ch || jx_signed_word(total)<0) return (JxFileWriteResult){0,total};
        uint8_t index=(uint8_t)(ch-32),category=0;
        if(index<=88) category=jx_ds(c)[0x156a+index]&15;
        state=jx_ds(c)[0x156a+(uint8_t)((category<<3)+state)]>>4;
        switch(state) {
        case 0:
            emitted=format_output(c,stream,workspace,ch,1,1,total,character,services);
            if(emitted.flow) return emitted;
            total=emitted.value;continue;
        case 1: prefix_count=width=uppercase=0;flags=0x20;precision=0xffff;continue;
        case 2:
            flags|=ch=='-'?4:ch=='+'?1:ch==' '?2:ch=='#'?128:8;continue;
        case 3:
            if(ch=='*') {
                width=format_argument(c,&arguments);
                if(jx_signed_word(width)<0) {width=(uint16_t)(0-width);flags|=4;}
            } else width=(uint16_t)(width*10+(uint8_t)(ch-'0'));
            continue;
        case 4: precision=0;continue;
        case 5:
            if(ch=='*') {
                precision=format_argument(c,&arguments);
                if(jx_signed_word(precision)<0) precision=0xffff;
            } else precision=(uint16_t)(precision*10+(uint8_t)(ch-'0'));
            continue;
        case 6:
            flags|=ch=='l'?0x10:ch=='F'?0x20:ch=='N'?0x1000:ch=='L'?0x400:0x800;
            continue;
        default: break; /* unmodified original table yields state 7 */
        }
        JxFarPointer text=workspace;uint16_t length=0;
        if(ch=='c') {
            jx_far_set_byte(c,text,0,(uint8_t)format_argument(c,&arguments));length=1;
        } else if(ch=='s') {
            text=format_pointer(c,&arguments,flags);
            if(!text.offset && !text.segment) {text=(JxFarPointer){0x15c3,c->data_segment};length=jx_word(c,0x15c9);}
            else length=format_length(c,text,precision);
        } else if(ch=='n') {
            text=format_pointer(c,&arguments,flags);format_word(c,text,0,total);
            if(flags&0x10) format_word(c,text,2,0);
            continue;
        } else if(ch=='p') {
            uint32_t value=(flags&0x30)?format_long(c,&arguments):format_argument(c,&arguments);
            if((flags&0x30) && !(flags&0x1800)) {
                (void)jx_format_digits(c,format_at(workspace,8),value&65535,16,4,7);
                (void)jx_format_digits(c,format_at(workspace,3),value>>16,16,4,7);
                jx_far_set_byte(c,workspace,4,':');length=9;
            } else {
                (void)jx_format_digits(c,format_at(workspace,3),value&65535,16,4,7);length=4;
            }
        } else if(ch=='d'||ch=='i'||ch=='u'||ch=='x'||ch=='X'||ch=='o') {
            uint16_t radix=(ch=='o')?8:(ch=='x'||ch=='X')?16:10;
            uint8_t adjustment=ch=='x'?39:7;
            if(ch=='d'||ch=='i') flags|=0x40;
            if(radix==16 && (flags&128)) {
                prefix_count=2;jx_far_set_byte(c,prefix,0,'0');jx_far_set_byte(c,prefix,1,ch);
            }
            if(radix==8 && (flags&128)) flags|=0x200;
            uint32_t value;
            if(flags&0x10) value=format_long(c,&arguments);
            else {
                uint16_t word=format_argument(c,&arguments);
                value=(flags&0x40)?(uint32_t)(int32_t)jx_signed_word(word):word;
            }
            if((flags&0x40) && (value&0x80000000u)) {flags|=0x100;value=0u-value;}
            if(jx_signed_word(precision)<0) precision=1;else flags&=(uint16_t)~8u;
            if(!value) prefix_count=0;
            JxFormatDigits digits=jx_format_digits(c,format_at(workspace,511),value,radix,precision,adjustment);
            text=digits.start;length=digits.count;
            if((flags&0x200) && (!length || jx_far_byte(c,text,0)!='0')) {
                text.offset--;jx_far_set_byte(c,text,0,'0');length++;
            }
        } else {
            if(ch=='E'||ch=='G') uppercase++;
            flags|=0x40;uint16_t conversion=(uint16_t)jx_signed_byte((uint8_t)(ch|32));
            if(jx_signed_word(precision)<0) precision=6;
            else if(!precision && conversion=='g') precision=1;
            uint16_t table=(flags&0x400)?0x15e8:0x15d4;
            (void)JX_CALL(c,jx_word(c,(uint16_t)(table+2)),jx_word(c,table),
                arguments.offset,arguments.segment,text.offset,text.segment,conversion,precision,uppercase);
            arguments.offset=(uint16_t)(arguments.offset+((flags&0x400)?10:8));
            if((flags&128) && !precision) format_float_call(c,0x15e0,text);
            if(conversion=='g' && !(flags&128)) format_float_call(c,0x15dc,text);
            if(jx_far_byte(c,text,0)=='-') {text.offset++;flags|=0x100;}
            length=format_length(c,text,65535);
        }
        if(flags&0x40) {
            uint8_t sign=(flags&0x100)?'-':(flags&1)?'+':(flags&2)?' ':0;
            if(sign) {jx_far_set_byte(c,prefix,0,sign);prefix_count=1;}
        }
        uint16_t padding=(uint16_t)(width-length-prefix_count);
        if(jx_signed_word(padding)<0) padding=0;
        if(!(flags&12)) {
            emitted=format_output(c,stream,workspace,' ',1,padding,total,character,services);
            if(emitted.flow) return emitted;
            total=emitted.value;
        }
        emitted=format_output(c,stream,prefix,0,0,prefix_count,total,character,services);
        if(emitted.flow) return emitted;
        total=emitted.value;
        if((flags&8) && !(flags&4)) {
            emitted=format_output(c,stream,workspace,'0',1,padding,total,character,services);
            if(emitted.flow) return emitted;
            total=emitted.value;
        }
        emitted=format_output(c,stream,text,0,0,length,total,character,services);
        if(emitted.flow) return emitted;
        total=emitted.value;
        if(flags&4) {
            emitted=format_output(c,stream,workspace,' ',1,padding,total,character,services);
            if(emitted.flow) return emitted;
            total=emitted.value;
        }
    }
}

uint16_t jx_sprintf_words(JxEventContext *c,JxFarPointer destination,
    JxFarPointer format,JxFarPointer arguments) {
    return sprintf_impl(c,destination,format,arguments,NULL);
}
uint16_t jx_sprintf_format(JxEventContext *c,JxFarPointer destination,
    JxFarPointer format,JxFarPointer arguments,JxFarPointer workspace) {
    return sprintf_impl(c,destination,format,arguments,&workspace);
}

uint16_t jx_format_words(JxEventContext *c,JxFarPointer stream,JxFarPointer format,JxFarPointer arguments,JxFarPointer workspace) {
    return format_words(c,stream,format,arguments,workspace,NULL).value;
}
JxFileWriteResult jx_format_words_dos(JxEventContext *c,JxFarPointer stream,JxFarPointer format,JxFarPointer arguments,JxFarPointer workspace,const JxCrtServices *services) {
    return format_words(c,stream,format,arguments,workspace,services);
}
