/* Parser/parser-generator */

#ifndef _Isyntax
#define _Isyntax 1

#include "tree.h"
#include "hash.h"

/* Syntax tree node
    */

struct action
{
    HASH *lits;		/* If there is a literal string at this point */
    struct action *expr;	/* If there is an expression at this point */
    struct action *space;	/* If there whitespace at this point */
    struct action *ospace;	/* Optional whitespace at this point */
    struct strlist *mac;	/* If this is end of input: macro to expand */
    struct strlist *list;	/* arguments to macro */
    char *src;		/* Source file info (where insn def is) */
};

/* A syntax pattern as returned by parsepattern() */

struct pattern
{
    struct pattern *next;	/* Next element of pattern */
    char *lit;		/* Literal string, patexpr, patwhite or patrule */
    struct rule *rule;	/* Syntax rule if 'lit' is patrule */
};

/* A syntax rule (or production) as returned by getrule() */

struct rule
{
    struct rule *next;		/* Next rule with this name (non-terminal) */
    struct pattern *pattern;	/* Pattern to match for this rule */
    struct strlist *list;		/* Result expression list to generate */
};

/* Parse an instruction definition */
void doinst(struct macro *macro,char *s);

/* Parse a syntax rule */
void dorule(struct macro *macro, char *s);

/* Parse an instruction.  Return true for failure. */
int doparse(char *s);

/* Show syntax */
void doshowsyntax(void);

#endif
