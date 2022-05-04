/* Assembly line parser and .if handling */

#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "frag.h"
#include "expr.h"
#include "macro.h"
#include "symtab.h"
#include "output.h"
#include "error.h"
#include "uasm.h"
#include "syntax.h"
#include "parse.h"

static int ifstate;		/* Current .if stack state */
/* 0=no .if block
            1=assemble current section
            2=skip until true .ifelse or .else
            3=skip until .endif
*/

/* .if stack */

struct ifelse
{
    struct ifelse *next;
    int state;
} *ifelse;

/* Skip whitespace */

char *parseskip(char *curline)
{
    for(;;) switch(*curline)
    {
        case ' ':
        case '\t':
            ++curline;
            break;

        case ';':
            *curline=0;
            return curline;

        case 0:
            return curline;

        default:
            return curline;
    }
    return curline;
}

/* Parse label.  Update ptr to point to first non-whitespace character after
    * label. */

char *parselabel(char **ptr)
{
    char *curline= *ptr;
    int x;
    for(x=0;curline[x] && curline[x]!=' ' && curline[x]!='\t' &&
                                    curline[x]!=':' && curline[x]!=';';++x);
    if(x)
    { /* We have a label */
        char *s=malloc(x+1);
        memcpy(s,curline,x);
        s[x]=0;
        if(curline[x]==':') ++x;
        *ptr=parseskip(curline+x);
        return s;
    }
    else
    {
        *ptr=parseskip(curline);
        return 0;
    }
}

/* Parse a field */

char *parsefield(char **ptr)
{
    char *curline= *ptr;
    int x;
    curline=parseskip(curline);
    for(x=0;curline[x] && curline[x]!=',' && curline[x]!=' ' && 
                                    curline[x]!='\t' && curline[x]!=';';++x);
    if(x)
    {
        char *s=malloc(x+1);
        memcpy(s,curline,x);
        s[x]=0;
        *ptr=parseskip(curline+x);
        return s;
    }
    else
    {
        *ptr=curline;
        return 0;
    }
}

static void popifelse(void)
{
    struct ifelse *next=ifelse->next;
    ifstate=ifelse->state;
    free(ifelse);
    ifelse=next;
}

static void dostate(char *s)
{
    Tree *v;
    s=parseskip(s);
    if(!*s)
    {
        error0("Missing argument for .if");
        ifstate=3;
        return;
    }
    v=expr(symtab,&s);
    v=simp(v);
    if(v->Int.id==Intid)
        if(v->Int.val) ifstate=1;
        else ifstate=2;
    else
    {
        error0("Arg for if must evaluate to an integer constant");
        ifstate=3;
    }
    rm(v);
    s=parseskip(s);
    if(*s)
        error0("junk after .if argument");
}

void doif(struct macro *macro, char *s)
{
    struct ifelse *new=malloc(sizeof(struct ifelse));
    new->next=ifelse;
    new->state=ifstate;
    ifelse=new;
    dostate(s);
}

void doelse(void)
{
    error0(".else without matching .if");
}

void doelseif(void)
{
    error0(".elseif without matching .if");
}

void doend(void)
{
    error0(".end without matching .macro, .insn, .foreach or .if");
}

void doifeof(void)
{
    if(ifstate) error0("Missing .endif");
}

/* Register a label */

static void dolabel(char *s)
{
    struct symbol *sym;
    if(!cursect) error1("No section for label '%s'",s);
    sym=findsym(symtab,s);
    if(sym->state) error1("Label '%s' multiply defined",s);
    else
    {
        char tmp[512];
        /* Define the label to our current location */
        sym->state=2;
        srcref(tmp);
        sym->defline=strdup(tmp);
        sym->v=mk(Labelid,cursect->last->len,cursect->last,cursect);
    }
}

/* Process mnemonic */

static void domnem(char *start,char *mnem,char *s)
{
    struct macro *op=findmacro(mnem);
    if(op) op->func(op,s);
    else if(doparse(start))
        error1("unknown mnemonic '%s'",mnem);
}

/* Assemble a source line */

void asmline(char *s)
{
    char *label=parselabel(&s);	/* Get label */
    char *start=s;			/* Start of mnemonic */
    char *mnem=parsefield(&s);	/* Get mnemonic */
    /* s points after mnemonic */

    /* If/elseif/else/end processing */
    switch(ifstate)
    {
        case 0:
            /* We are not in a .if block */
        {
            if(label) dolabel(label);		/* Handle label */
            if(mnem) domnem(start,mnem,s);	/* Handle instruction */
            break;
        }

        case 1:
            /* Assemble current section.  Skip to .end once we hit
                        .elseif or .else.  Pop if stack when .end is found. */
        {
            if(label) dolabel(label);	/* Handle label */
            if(mnem)
                if(!strcmp(mnem,".elseif") || !strcmp(mnem,".else")) ifstate=3;
                else if(!strcmp(mnem,".end")) popifelse();
                else domnem(start,mnem,s);	/* Handle instruction */
            break;
        }

        case 2:
            /* Skip until true ifelse or else is found, or pop when
                        .end is found */
        {
            if(mnem)
                if(!strcmp(mnem,".else"))
                {
                    if(label) dolabel(label);
                    ifelse->state=1;
                }
                else if(!strcmp(mnem,".elseif"))
                {
                    if(label) dolabel(label);
                    dostate(s);
                }
                else if(!strcmp(mnem,".end"))
                {
                    if(label) dolabel(label);
                    popifelse();
                }
            break;
        }

        case 3:
            /* Skip until endif is found */
        {
            if(mnem && !strcmp(mnem,".end"))
            {
                if(label) dolabel(label);
                popifelse();
            }
            break;
        }
    }
    if(label) free(label);
    if(mnem) free(mnem);
}
