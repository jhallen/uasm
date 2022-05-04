/* Uasm header file */

#ifndef _Iuasm
#define _Iuasm 1

#define LINESZ 256

/* Directives */

struct macro;

void doalign(struct macro *mac, char *s);
void dospace(struct macro *mac, char *s);
void doend(void);
void doelseif(void);
void doelse(void);
void doif(struct macro *mac, char *s);
void doset(struct macro *mac, char *s);
void doequ(struct macro *mac, char *s);
void dosect(struct macro *mac, char *s);
void dopublic(struct macro *mac, char *s);
void doemit(struct macro *mac, char *s);
void doforeach(struct macro *mac, char *s);
void doerrif(struct macro *mac, char *s);
void dosmall(struct macro *mac, char *s);
void dobigger(struct macro *mac, char *s);
void dobiggest(struct macro *mac, char *s);
void dodone(struct macro *mac, char *s);

extern char *afields[];
extern char parseline[];

#endif
