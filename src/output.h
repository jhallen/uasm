#ifndef _Ioutput
#define _Ioutput

extern FILE *emitfile;
void objrec(int type);
void endrec(void);
void emitblk(char *blk,int len);
void emitc(int c);
void emits(char *s);
void emitsn(char *s, int len);
void izemit(void);
void emitnum(unsigned long n);

#endif
