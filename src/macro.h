/* Input reader & macro expansion */

#ifndef _Imacro
#define _Imacro 1

#include "hash.h"

struct source
{
    struct source *next;	/* Source to use when this one is exhausted */
    int line;		/* Current source line no. */
    char *name;		/* Current source name */
    FILE *file;		/* If source is a file.  NULL if source is a macro */
    struct strlist *mac;	/* Next macro line */
    HASH *args;		/* Hash table of 'struct subst's (arg substitutes) */
};

/* Built-in assembler directive or macro */

struct macro
{
    char *name;		/* Name of directive or macro */
    void (*func)();	/* Function to assemble directive or expand macro */
    struct strlist *body;	/* Lines which make up body of macro */
    struct strlist *args;	/* Argument list */
};

extern int gline;		/* Global line no. */

char *getsource(void);		/* Get next source line */
void pushfile(char *s,int flg);
void pushm(char *name,struct strlist *body,HASH *args);
void addpath(char *s);
/* Lookup assembler directive */
struct macro *findmacro(char *s);
struct strlist *record(void);	/* Record lines until .end */

#endif
