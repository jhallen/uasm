/* Intermediate file output functions */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "output.h"

FILE *emitfile;
long rec_addr;

char *recbuf;
int reclen;
int recsiz;
int rectype;

void izemit(void)
{
    recbuf=malloc(recsiz=1024);
    reclen=0;
    /* emitfile=stdout; */
}

void objrec(int type)
{
    if(!recbuf) izemit();
    rectype=type;
}

void endrec(void)
{
    int len=reclen;
    emitnum(len);
    fputc(rectype,emitfile);
    fwrite(recbuf+len,1,reclen-len,emitfile);
    if(len) fwrite(recbuf,1,len,emitfile);
    reclen=0;
}

void emitblk(char *blk,int len)
{
    if(len)
    {
        if(reclen+len>recsiz)
            recbuf=realloc(recbuf,recsiz=(reclen+len)*2);
        memcpy(recbuf+reclen,blk,len);
        reclen+=len;
    }
}

void emitc(int c)
{
    if(reclen+1>recsiz)
        recbuf=realloc(recbuf,recsiz*=2);
    recbuf[reclen++]=c;
}

void emits(char *s)
{
    emitnum(strlen(s)+1);
    emitblk(s,strlen(s)+1);
}

void emitsn(char *s, int len)
{
    emitnum(len+1);
    emitblk(s,len+1);
}

void emitnum(unsigned long n)
{
    if(n<126) emitc(128+n);
    else if(n<32768L) emitc(n>>8), emitc(n);
    else
        emitc(255),
            emitc((n>>24)&255),
            emitc((n>>16)&255),
            emitc((n>>8)&255),
            emitc(n&255);
}
