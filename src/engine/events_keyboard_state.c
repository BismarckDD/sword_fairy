#include "event_engine.h"

/* 1F2A:6DAC. Negative directions consume no random number. The first
 * signed remainder >40 retains even an out-of-range nonnegative direction.
 * Otherwise only directions0..3 consume a second draw and turn perpendicular. */
uint16_t jx_keyboard_randomize_direction(JxEventContext *c,uint16_t direction) {
    if(jx_signed_word(direction)<0)return 0xffff;
    int first=jx_signed_word((uint16_t)JX_CALL0(c,0x1000,0x32b2))%100;
    if(first>40)return direction;
    if(direction>3)return 0xffff;
    int second=jx_signed_word((uint16_t)JX_CALL0(c,0x1000,0x32b2))%2;
    return (uint16_t)(second+(direction<2?2:0));
}
/* 1F2A:6E62. A signed-positive player status enables randomized input;
 * query uses the current first direction after reading that status. */
uint16_t jx_keyboard_read_direction(JxEventContext *c) {
    uint16_t direction=jx_word(c,0x2942);
    if(jx_signed_word(jx_word(c,0xd0fa))>0)
        return (uint16_t)JX_CALL(c,0x1f2a,0x6dac,direction);
    return direction;
}

/* 1F2A:6EB8. Two live segment aliases supply sentinel and source codes;
 * translated byte explicitly comes from original fixed segment 39E3.
 * Scan argument uses its low byte, counter and offsets wrap as words. */
uint8_t jx_keyboard_translate(JxEventContext *c,uint16_t scan) {
    for(uint16_t index=0;;index=(uint16_t)(index+1)) {
        uint16_t offset=(uint16_t)(0x04f4+index*2u);
        if(jx_far_byte(c,(JxFarPointer){offset,jx_word(c,0x2394)},0)==0)return 0;
        if(jx_far_byte(c,(JxFarPointer){offset,jx_word(c,0x2396)},0)==(uint8_t)scan)
            return jx_far_byte(c,(JxFarPointer){(uint16_t)(offset+1),0x39e3},0);
    }
}
/* 1F2A:6F20. Four-direction pressed state and most-recent-first priority.
 * Keep sequential stores, including malformed direction/table aliases. */
uint16_t jx_keyboard_set_direction(JxEventContext *c,uint16_t direction,
    uint16_t pressed) {
    uint16_t state=(uint16_t)(0x293a+direction*2u);
    if(pressed!=0) {
        if(jx_word(c,state)!=0)return 0;
        jx_set_word(c,0x2948,jx_word(c,0x2946));
        jx_set_word(c,0x2946,jx_word(c,0x2944));
        jx_set_word(c,0x2944,jx_word(c,0x2942));
        jx_set_word(c,0x2942,direction);jx_set_word(c,state,1);
        return 1;
    }
    if(jx_word(c,state)==0)return 0;
    unsigned index=0;
    while(index<4 && jx_word(c,(uint16_t)(0x2942+2*index))!=direction)index++;
    if(index==4)return 0;
    jx_set_word(c,state,0);
    for(unsigned i=index;i<3;i++)
        jx_set_word(c,(uint16_t)(0x2942+2*i),jx_word(c,(uint16_t)(0x2944+2*i)));
    jx_set_word(c,0x2948,0xffff);
    return 1;
}
