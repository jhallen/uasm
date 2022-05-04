/* Symbol table */

#ifndef _Isymtab
#define _Isymtab 1

#include "tree.h"
#include "hash.h"

/* A symbol table */

struct symtab
{
    HASH *symtab;
};

extern struct symtab symtab[];	/* Root symbol table */

/* A symbol in the symbol table */

struct symbol
{
    char *name;		/* Label name */
    int no;		/* Symbol no. */
    int state;		/* 0 for undefined, 1 if set, 2 if equated */
    int pub;		/* Set if this is a public symbol */
    Tree *v;		/* Value of symbol.  NULL for undefined. */

    /* For use in assembler */
    char *defline;		/* Line where symbol was defined */
    struct strlist *lines;	/* Lines which reference this symbol */

    /* For use in linker */
    struct module *module;	/* Module where this symbol was defined */
    struct module **ref;	/* List of references to this symbol */
    unsigned nref;
    unsigned bksize;
};

extern unsigned nlabels;	/* No. of labels */

/* struct symbol *findsym(char *s);
    * Look up a symbol
    */
struct symbol *findsym(struct symtab *symtab, char *s);

/* Add reference to symbol.
    */
extern void addref();

void addlinkref(struct symbol *symbol, struct module *module);

void emitsymtab(void);
void emitsymbol(struct symbol *sym);
void xreflisting(FILE *file);

void linkxref(FILE *file);


#endif
