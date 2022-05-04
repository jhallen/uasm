#ifndef _Istrlist
#define _Istrlist 1

#include "hash.h"

/* Linked list of strings */

struct strlist
{
    struct strlist *next;	/* Next string */
    char *str;		/* The string */
};

/* String substitute */

struct subst
{
    char *name;		/* Name */
    char *subst;		/* String to substitute for name */
};

struct strlist *mkstrlist(struct strlist *next,char *str);
char *stringexpr(char **ptr);
struct strlist *parselist(char **ptr);
char *mkexprno(int n);
struct strlist *xformlist(struct strlist *input, struct strlist *pattern);
struct strlist *appendlist(struct strlist *list, struct strlist *item);
struct strlist *appenditem(struct strlist *list, char *item);
struct strlist *duplist(struct strlist *list);
struct strlist *reverselist(struct strlist *str);
void showlist(struct strlist *l);
void rmstrlist(struct strlist *l);
HASH *mkhash(struct strlist *list);
void rmhash(HASH *t);
void substcopy(char *dest,char *src,HASH *args);

#endif
