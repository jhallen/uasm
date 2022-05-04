#ifndef _Iinput
#define _Iinput 1

extern unsigned char *recbuf;
extern int reclen;
extern int recidx;
extern int getrec(FILE *fd);
extern char *getword(void);


/* Get character */

#define GETC() recbuf[recidx++]

/* Get variable length number from buf indexed by x.  Increment x past
            number */

#define GETNUM() \
    ( \
    recbuf[recidx]==255 ? \
        (recidx+=5, (recbuf[recidx-4]<<24)+(recbuf[recidx-3]<<16)+ \
                                                        (recbuf[recidx-2]<<8)+(recbuf[recidx-1])) \
    : \
        recbuf[recidx]>=128 ? \
            (recidx+=1, (recbuf[recidx-1]-128)) \
        : \
            (recidx+=2, (recbuf[recidx-2]<<8)+(recbuf[recidx-1])) \
    )

#endif
