#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "expr.h"
#include "strlist.h"

/* Function copies string from src to dest, and does macro
    * substitution along the way. */

void substcopy(char *dest,char *src,HASH *args)
{
    int x, y;
    char buf[256];
    x=y=0;
    while(src[x])
        /* Is character start of a word? */
        if(src[x]>='A' && src[x]<='Z' || src[x]>='a' && src[x]<='z' ||
                    src[x]=='_')
        { /* It is, check if we should substitute */
            int q=x;
            struct subst *mac;
            /* Find end of word */
            while(src[q]>='A' && src[q]<='Z' || src[q]>='a' && src[q]<='z' ||
                                    src[q]>='0' && src[q]<='9' || src[q]=='_') ++q;
            /* Lookup word in arg table */
            if(mac=htfindn(args,hashn(src+x,q-x),src+x,q-x))
            { /* Found it: copy in the substitute */
                x=q;
                dest[y++]='(';
                for(q=0;mac->subst[q];++q) dest[y++]=mac->subst[q];
                dest[y++]=')';
            }
            else
                /* No substitution: copy word directly */
                while(x!=q) dest[y++]=src[x++];
        }
        else if(src[x]=='$' && src[x+1]=='(')
        {
            struct subst *mac;
            int q;
            x+=2;
            q=x;
            while(src[q] && src[q]!=')') ++q;
            if(src[q]==')' && q!=x && (mac=htfindn(args,hashn(src+x,q-x),src+x,q-x)))
            {
                x=q+1;
                for(q=0;mac->subst[q];++q) dest[y++]=mac->subst[q];
            }
            else
            {
                x-=2;
                ++q;
                while(x!=q) dest[y++]=src[x++];
            }
        }
        else if(src[x]=='\"') /* Encountered a string? */
            do dest[y++]=src[x++]; while(src[x] && !(src[x]=='"' && src[x-1]!='\\'));
        else if(src[x]=='\'') /* Encountered a character constant? */
            do dest[y++]=src[x++]; while(src[x] && !(src[x]=='\'' && src[x-1]!='\\'));
        else
            /* Simple copy */
            dest[y++]=src[x++];
    dest[y]=0;
}

struct strlist *mkstrlist(struct strlist *next,char *str)
{
    struct strlist *n=malloc(sizeof(struct strlist));
    n->next=next;
    n->str=str;
    return n;
}

/* Parse an expression, but return string containing expression instead of
            parse tree. */

char *stringexpr(char **ptr)
{
    char *org= *ptr;
    Tree *n=expr(NULL,ptr);
    if(n)
    {
        char *rtn;
        char *eos;
        rm(n);
        /* Kill leading and trailing whitespace */
        while(*org==' ' || *org=='\t') ++org;
        eos= *ptr;
        while(eos!=org && (eos[-1]==' ' || eos[-1]=='\t')) --eos;
        rtn=malloc(1+eos-org);
        memcpy(rtn,org,eos-org);
        rtn[eos-org]=0;
        return rtn;
    }
    else
    {
        *ptr=org;
        return 0;
    }
}

/* Parse an expression list */

struct strlist *parselist(char **ptr)
{
    struct strlist *first, *last;
    char *exp;
    for(first=last=0;exp=stringexpr(ptr);)
    {
        struct strlist *n=mkstrlist(NULL,exp);
        if(last) last->next=n, last=n;
        else first=last=n;
        while(**ptr==' ' || **ptr=='\t') ++*ptr;
        if(**ptr==',') ++*ptr;
        else break;
    }
    return first;
}

struct strlist *reverselist(struct strlist *str)
{
    struct strlist *last, *next;
    for(last=0;str;str=next)
    {
        next=str->next;
        str->next=last;
        last=str;
    }
    return last;
}

/* Create string representation of argument no.  This is for when we see
            <expr> in a pattern, which refers to an input expression. */

char *mkexprno(int n)
{
    char buf[10];
    sprintf(buf,"arg%d",n);
    return strdup(buf);
}

struct strlist *xformlist(struct strlist *input, struct strlist *pattern)
{
    struct strlist *first, *last;
    HASH *t=mkhash(input);
    for(first=last=0;pattern;pattern=pattern->next)
    {
        char buf[1024];
        struct strlist *new;
        substcopy(buf,pattern->str,t);
        new=mkstrlist(NULL,strdup(buf));
        if(last) last->next=new, last=new;
        else first=last=new;
    }
    rmhash(t);
    return first;
}

/* Append list to end of other list */

struct strlist *appendlist(struct strlist *list, struct strlist *item)
{
    struct strlist *last;
    if(!list) return item;
    for(last=list;last->next;last=last->next);
    last->next=item;
    return list;
}

/* Append item to end of list */

struct strlist *appenditem(struct strlist *list, char *item)
{
    return appendlist(list,mkstrlist(NULL,item));
}

/* Duplicate list */

struct strlist *duplist(struct strlist *list)
{
    struct strlist *first, *last;
    for(first=last=0;list;list=list->next)
    {
        struct strlist *l=mkstrlist(NULL,strdup(list->str));
        if(last) last->next=l;
        else first=last=l;
    }
    return first;
}

void showlist(struct strlist *l)
{
    while(l)
    {
        printf("[%s] ",l->str);
        l=l->next;
    }
    printf("\n");
}

void rmstrlist(struct strlist *l)
{
    struct strlist *n;
    while(l)
    {
        n=l->next;
        if(l->str) free(l->str);
        free(l);
        l=n;
    }
}

/* Delete substitution hash table */

void rmsubst(void *obj,char *name,void *v)
{
    struct subst *val = (struct subst *)v;
    free(val->name);
    free(val->subst);
    free(val);
}

void rmhash(HASH *t)
{
    if(t)
    {
        htall(t,rmsubst,NULL);
        htrm(t);
    }
}

/* Convert list into substitution hash table */

HASH *mkhash(struct strlist *list)
{
    struct strlist *l;
    HASH *t;
    int n;
    t=htmk(32);
    n=0;
    for(l=list;l;l=l->next)
    {
        char buf[20];
        struct subst *subst=malloc(sizeof(struct subst));
        sprintf(buf,"arg%d",++n);
        subst->name=strdup(buf);
        subst->subst=strdup(l->str);
        htadd(t,subst->name,subst);
    }
    return t;
}
