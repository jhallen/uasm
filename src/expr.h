/* Expression parser
    */

#ifndef _Iexpr
#define _Iexpr 1

/* Types */

typedef struct opr OPR;

#include "tree.h"

/* Operator descriptor table entry */

struct opr
{
    char *name;		/* How it's scanned or printed */
    OPR *prefix;		/* Context dependant alternatives */
    OPR *infix;
    OPR *postfix;
    int prec;		/* Precedence */
    int ass;		/* Set for right-associative; clear for left */
    int meth;		/* Method for building parse-tree */
    void *func;		/* Function name or tree node */
};

/* Tree *expr(struct symtab *,char **s);
            Convert expression in string into a parse-tree.
            Updates string pointer to after expression.
            */
struct symtab;
Tree *expr(struct symtab *symtab, char **s);

#endif
