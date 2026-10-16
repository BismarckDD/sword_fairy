#include "event_engine.h"
#include <stdio.h>

static uint16_t board_get(void *opaque) {
    int value=fgetc(opaque);return value<0?65535:(uint8_t)value;
}
static void board_unget(void *opaque,uint16_t value) {(void)ungetc((uint8_t)value,opaque);}
static void scan_word(FILE *file,JxEventContext *c,uint16_t destination) {
    uint16_t value;JxScanInput input={file,board_get,board_unget};
    if(jx_scan_decimal16(c,&input,&value)==1)jx_set_word(c,destination,value);
}
static void scan_token(FILE *file,JxEventContext *c,char *token) {
    JxScanInput input={file,board_get,board_unget};
    (void)jx_scan_token(c,&input,(uint8_t *)token,127);
}

/* Overlay 002:0000. Resource names are relative to the game's directory.
 * The immutable format/path strings at DS:01B8..01E7 were recovered from
 * the original image. Valid resources have tokens shorter than 128 bytes;
 * cap %s here rather than reproduce the original stack-buffer overflow.
 * EOF deliberately preserves the previous token and does not end the fixed
 * 300-command loop. Return values match AX: 0 success, -1 open failure. */
int jx_board_load(JxEventContext *c,int16_t resource) {
    char number[8],name[48],token[128];
    (void)snprintf(number,sizeof(number),"%2d",(int)resource);
    if(number[0]==' ') number[0]='0';
    number[2]='\0';
    (void)snprintf(name,sizeof(name),"go%s.txt",number);
    FILE *file=fopen(name,"r");
    if(!file) return -1;
    scan_token(file,c,token);
    for(unsigned row=0;row<19;row++) {
        scan_token(file,c,token);
        for(unsigned column=0;column<19;column++)
            /* Original %d writes a WORD despite a BYTE-indexed board. */
            scan_word(file,c,(uint16_t)(0x5866+19*row+column));
    }
    scan_word(file,c,0x3462);scan_word(file,c,0x3464);
    (void)fclose(file);
    (void)snprintf(name,sizeof(name),"go%s.go",number);
    file=fopen(name,"r");
    if(!file) return -1;
    for(unsigned index=0;index<300;index++) {
        uint16_t command=(uint16_t)(0x4176+index*6);
        scan_token(file,c,token);
        if(token[0]=='B') {
            jx_set_word(c,(uint16_t)(command+4),1);
            scan_token(file,c,token);
        } else if(token[0]=='W') {
            jx_set_word(c,(uint16_t)(command+4),2);
            scan_token(file,c,token);
        } else jx_set_word(c,(uint16_t)(command+4),token[0]=='P'?3:0);
        scan_token(file,c,token);
        int column=jx_signed_byte((uint8_t)token[0])-'A';
        if(column>8) column--; /* Go coordinates omit I after H. */
        jx_set_word(c,command,(uint16_t)column);
        int row=jx_signed_byte((uint8_t)token[1])-'1';
        if(token[2]!=0) row=(jx_signed_byte((uint8_t)token[1])-'0')*10+
            jx_signed_byte((uint8_t)token[2])-'1';
        jx_set_word(c,(uint16_t)(command+2),(uint16_t)(18-row));
    }
    (void)fclose(file);
    return 0;
}
