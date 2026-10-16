#include "event_engine.h"

/* Shared disk path of 1F2A:01E4 and 0350. Errors are accumulated, not early
 * returns: even a null fopen result is passed to seek/read/close, as in the
 * original. A later seek/short-read error supersedes the open-error code. */
static int font_from_file(JxEventContext *c,uint16_t filename,uint16_t mode,
    uint16_t source_name,int16_t index,unsigned bytes) {
    JxFarPointer destination={filename,c->data_segment},source={source_name,c->data_segment};
    (void)jx_string_copy(c,destination,source);
    uint32_t file=JX_CALL(c,0x1000,0x0738,filename,c->data_segment,mode,c->data_segment);
    jx_set_word(c,0x2844,(uint16_t)file);jx_set_word(c,0x2846,(uint16_t)(file>>16));
    int error=file==0?1:0;
    uint32_t position=(uint32_t)((int32_t)index*(int32_t)bytes);
    if((uint16_t)JX_CALL(c,0x1000,0x27fa,jx_word(c,0x2844),jx_word(c,0x2846),
        (uint16_t)position,(uint16_t)(position>>16),0)!=0) error=2;
    if((uint16_t)JX_CALL(c,0x1000,0x0754,0x2724,c->data_segment,1,(uint16_t)bytes,
        jx_word(c,0x2844),jx_word(c,0x2846))!=bytes) error=2;
    (void)JX_CALL(c,0x1000,0x0636,jx_word(c,0x2844),jx_word(c,0x2846));
    return error;
}

/* 1F2A:01E4. The packed double-byte character selects a 94-column glyph
 * grid. EMS holds 512 32-byte glyphs per logical page, starting at page 1.
 * Unlike the single-byte loader, this path ignores EMS mapping failure. */
int jx_font_load_double(JxEventContext *c,uint16_t character) {
    int row=(character>>8)-0xa1,column=(character&0x7f)-0x21;
    uint16_t raw_index=(uint16_t)(row*94+column);
    int16_t index=(int16_t)jx_signed_word(raw_index);
    if(jx_word(c,0x004a)!=0) {
        int page=index>=0?index/512:-((-index+511)/512);
        uint16_t offset=(uint16_t)((raw_index&511)*32);
        (void)JX_CALL(c,0x1503,0x007d,0,(uint16_t)(page+1),jx_word(c,0x9332));
        JxFarPointer destination={0x2724,c->data_segment};
        JxFarPointer source={(uint16_t)(jx_word(c,0x5858)+offset),jx_word(c,0x585a)};
        (void)jx_memory_copy(c,destination,source,32);
        return 0;
    }
    return font_from_file(c,0x8742,0x0073,jx_word(c,0x3456)!=0?0x0069:0x006d,index,32);
}

/* 1F2A:0350. A signed character index selects a 16-byte glyph; cached
 * glyphs occupy EMS logical page 0. Adding to the far base wraps offset. */
int jx_font_load_single(JxEventContext *c,int16_t character) {
    if(jx_word(c,0x004a)!=0) {
        uint16_t offset=(uint16_t)((int32_t)character*16);
        if((uint16_t)JX_CALL(c,0x1503,0x007d,0,0,jx_word(c,0x9332))!=0) return -1;
        JxFarPointer destination={0x2724,c->data_segment};
        JxFarPointer source={(uint16_t)(jx_word(c,0x5858)+offset),jx_word(c,0x585a)};
        (void)jx_memory_copy(c,destination,source,16);
        return 0;
    }
    return font_from_file(c,0x931c,0x0083,jx_word(c,0x3456)!=0?0x0076:0x007d,character,16);
}
