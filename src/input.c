/* Intermediate file input functions */

#include <stdio.h>
#include <stdlib.h>
#include "input.h"

struct symbol **sym;
struct section **sec;

unsigned char *recbuf;	/* Record buffer */
int reclen;		/* Record length */
int recidx;		/* Record index */
static int recbufsize;	/* Malloc size of record buffer */

/* Get variable length number (use GETNUM normally) */

int getnum(FILE *fd)
{
    int c=fgetc(fd);
    if(c==255)
    {
        int d=fgetc(fd);
        int e=fgetc(fd);
        int f=fgetc(fd);
        int g=fgetc(fd);
        return (d<<24)+(e<<16)+(f<<8)+(g);
    }
    else if(c>=128)
        return c-128;
    else
    {
        int d=fgetc(fd);
        return (c<<8)+(d);
    }
}

/* Get a record */

int getrec(FILE *fd)
{
    int type=fgetc(fd);

    reclen=getnum(fd);

    if(reclen>recbufsize)
        if(!recbuf) recbuf=malloc(recbufsize=(reclen>1024?reclen*2:1024));
        else recbuf=realloc(recbuf,recbufsize= reclen*2);
    fread(recbuf,1,reclen,fd);
    recidx=0;
    return type;
}

/* Get word */

char *getword(void)
{
    int len=GETNUM();
    char *s=recbuf+recidx;
    recidx+=len;
    return s;
}
