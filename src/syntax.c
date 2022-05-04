/* Parser-generator & parser */

/* todo: add showrules.
                                    add showsymboltable.
    */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#include "error.h"
#include "parse.h"
#include "macro.h"
#include "strlist.h"
#include "syntax.h"

/* Syntax tree */

struct action root[1];

/* Syntax rules */

HASH *rules;

/* Get rule list with given name */

/* Head node of each rule list does not contain a pattern- instead it is
    * just a place holder for the head of the list.  This way we can refer to
    * named rules before they are defined (when a rule is defined, it is inserted
    * after the head node).
    */

struct rule *getrule(char *name)
{
    struct rule *rule;
    if(!rules) rules=htmk(256);
    if(rule=htfind(rules,name)) return rule;
    rule=malloc(sizeof(struct rule));
    rule->next=0;
    rule->pattern=0;
    rule->list=0;
    htadd(rules,name,rule);
    return rule;
}

/* Pattern list parser */

/* Atoms for lit field of 'struct pattern' */
char patexpr[]="expr";		/* atom for parse an expression */
char patwhite[]="space";	/* atom for parse whitespace */
char patowhite[]="ospace";	/* atom for optional whitespace */
char patrule[]="rule";		/* atom for parse a rule */

/* If lit field is not an atom, it is a literal string to match */

/* Parse single pattern */

struct pattern *parsepat(char **ptr)
{
    struct pattern *pat=0;
    char *line= *ptr;
    switch(*line)
    {
        case 0: case ';':
            break;

        case ' ': case '\t':
        {
            while(*line==' ' || *line=='\t') ++line;
            if(*line && *line!=';')
            {
                pat=malloc(sizeof(struct pattern));
                pat->next=0;
                pat->lit=patwhite;
                pat->rule=0;
            }
            break;
        }

        case '<':
        {
            char *s=++line;
            while(*line && *line!='>') ++line;
            if(!*line) error0("Missing > in rule name");
            if(line-s)
            {
                char *name=malloc(line-s+1);
                memcpy(name,s,line-s);
                name[line-s]=0;
                pat=malloc(sizeof(struct pattern));
                pat->next=0;
                if(!strcmp(name,"expr")) pat->lit=patexpr, pat->rule=0;
                else pat->lit=patrule, pat->rule=getrule(name);
            }
            else
            {
                pat=malloc(sizeof(struct pattern));
                pat->next=0;
                pat->lit=patowhite;
                pat->rule=0;
            }
            if(*line=='>') ++line;
            break;
        }

        default:
        {
            char *s=line;
            pat=malloc(sizeof(struct pattern));
            pat->next=0;
            while(*line && *line!=';' && *line!='<' && *line!=' ' && *line!='\t') ++line;
            pat->lit=malloc(line-s+1);
            memcpy(pat->lit,s,line-s);
            pat->lit[line-s]=0;
            pat->rule=0;
            break;
        }
    }
    *ptr=line;
    return pat;
}

/* Parse pattern list */

struct pattern *parsepattern(char **ptr)
{
    struct pattern *first, *last, *n;
    *ptr=parseskip(*ptr);
    for(first=last=0;n=parsepat(ptr);)
        if(last) last->next=n, last=n;
        else first=last=n;
    return first;
}

/* Free pattern list */

void rmpattern(struct pattern *p)
{
    while(p)
    {
        struct pattern *n=p->next;
        if(p->lit!=patexpr && p->lit!=patwhite && p->lit!=patowhite && p->lit!=patrule)
            free(p->lit);
        free(p);
        p=n;
    }
}

/* Assembler pseudo-op: Parse a syntax rule */

void dorule(struct macro *macro, char *s)
{
    struct rule *rule, *new;
    char *name;
    struct strlist *result;
    struct pattern *pattern;
    name=parsefield(&s);
    if(!name)
    {
        error0("need rule name in syntax rule");
        return;
    }
    result=parselist(&s);
    if(!result)
    {
        error0("need result list in syntax rule");
        return;
    }
    pattern=parsepattern(&s);
    if(!pattern)
    {
        error0("need syntax pattern in syntax rule");
        return;
    }
    /* Ok, we've got everything we need.  Add the rule */
    rule=getrule(name);
    new=malloc(sizeof(struct rule));
    new->pattern=pattern;
    new->list=result;
    new->next=rule->next;
    rule->next=new;
}

struct actionlist
{
    struct actionlist *next;
    struct action *a;
    struct strlist *l;
    int exprno;
};

struct actionlist *eol(struct actionlist *l)
{
    while(l->next) l=l->next;
    return l;
}

void rmactionlist(struct actionlist *l)
{
    while(l)
    {
        struct actionlist *n=l->next;
        free(n);
        l=n;
    }
}

/* Add action(s) corresonding to given pattern.  A list of the added actions
            is returned. */

static char srcinfo[80];

struct actionlist *addaction(struct action *action,struct pattern *pat,struct strlist *mac,struct strlist *list,int exprno)
{
    if(!pat)
    {
        struct actionlist *l;
        if(mac)
        {
            if(action->mac)
            {
                error("duplicate instruction definition");
            }
            else
            {
                action->mac=mac;
                action->list=list;
            }
        }
        l=malloc(sizeof(struct actionlist));
        l->next=0;
        l->a=action;
        l->exprno=exprno;
        if(mac) l->l=0;
        else l->l=list;
        return l;
    }
    else if(pat->lit==patexpr)
    {
        struct action *a;
        if(!(a=action->expr))
        {
            a=malloc(sizeof(struct action));
            a->src=strdup(srcinfo);
            a->expr=0;
            a->space=0;
            a->ospace=0;
            a->mac=0;
            a->lits=htmk(256);
            action->expr=a;
        }
        list=appenditem(list,mkexprno(exprno));
        return addaction(a,pat->next,mac,list,exprno+1);
    }
    else if(pat->lit==patwhite)
    {
        struct action *a;
        if(!(a=action->space))
        {
            a=malloc(sizeof(struct action));
            a->src=strdup(srcinfo);
            a->expr=0;
            a->space=0;
            a->ospace=0;
            a->mac=0;
            a->lits=htmk(256);
            action->space=a;
        }
        return addaction(a,pat->next,mac,list,exprno);
    }
    else if(pat->lit==patowhite)
    {
        struct action *a;
        if(!(a=action->ospace))
        {
            a=malloc(sizeof(struct action));
            a->src=strdup(srcinfo);
            a->expr=0;
            a->space=0;
            a->ospace=0;
            a->mac=0;
            a->lits=htmk(256);
            action->ospace=a;
        }
        return addaction(a,pat->next,mac,list,exprno);
    }
    else if(pat->lit==patrule)
    {
        struct rule *rule;
        struct actionlist *first=0, *last=0;
        for(rule=pat->rule->next;rule;rule=rule->next)
        {
            struct actionlist *q=addaction(action,rule->pattern,NULL,list,exprno);
            struct actionlist *r;
            for(r=q;r;r=r->next)
            {
                struct actionlist *n;
                n=addaction(r->a,pat->next,mac,
                                                                appendlist(duplist(list),xformlist(r->l,rule->list)),
                                                                r->exprno);
                if(last) last=eol(last->next=n);
                else last=eol(first=n);
            }
        }
        return first;
    }
    else
    {
        struct action *a;
        if(!(a=htfind(action->lits,pat->lit)))
        {
            a=malloc(sizeof(struct action));
            a->src=strdup(srcinfo);
            a->expr=0;
            a->space=0;
            a->ospace=0;
            a->mac=0;
            a->lits=htmk(256);
            htadd(action->lits,strdup(pat->lit),a);
        }
        return addaction(a,pat->next,mac,list,exprno);
    }
}

/* Assembler pseudo-op: Parse an instruction definition */

void doinst(struct macro *macro,char *s)
{
    struct strlist *first;
    struct pattern *pattern;
    if(source) sprintf(srcinfo,"%s+%d",source->name,source->line);
    else sprintf(srcinfo,"Huh?");
    pattern=parsepattern(&s);	/* Parse pattern */
    if(!pattern)
        error0("need syntax pattern in instruction definition");
    if(!root->lits) root->lits=htmk(256);
    first=record();
    /* Generate syntax tree from pattern */
    if(pattern)
    {
        addaction(root,pattern,first,NULL,1);
        rmpattern(pattern);
    }
}

/* Show syntax tree for debugging */

void showtree(int lvl,char *name,struct action *t)
{
    if(t)
    {
        indent(stdout,lvl); printf("%s\n",name);
        if(t->lits) htall(t->lits,showtree,lvl+1);
        showtree(lvl+1,"expr",t->expr);
        showtree(lvl+1,"space",t->space);
        showtree(lvl+1,"ospace",t->ospace);
    }
}

void doshowsyntax(void)
{
    showtree(0,"root",root);
}

/* Backtracking line parser */

struct strlist *stack;

struct action *parseitem(struct action *item,char *s)
{
    char *org=s;
    struct action *result;
    struct action *a;
    char *n;
    if(!*s)
        if(item->mac) return item;
        else return 0;
/*
    if(result=htlfindx(item->lits,&s,parseitem)) return result;
    s=org;
*/
    if(a=htlfind(item->lits,&s))
    {
        if(result=parseitem(a,s)) return result;
        s=org;
    }
    if(item->space && (*s==' ' || *s=='\t'))
    {
        while(*s==' ' || *s=='\t') ++s;
        if(result=parseitem(item->space,s)) return result;
        else s=org;
    }
    if(item->ospace)
    {
        while(*s==' ' || *s=='\t') ++s;
        if(result=parseitem(item->ospace,s)) return result;
        else s=org;
    }
    if(item->expr && (n=stringexpr(&s)))
    {
        if(result=parseitem(item->expr,s))
        {
            stack=mkstrlist(stack,n);
            return result;
        }
        else org=s, free(n);
    }
    return 0;
}

/* Parse assembly language source line using syntax tree */

int doparse(char *s)
{
    struct action *result;
    struct strlist *mac;
    if(!root->lits) root->lits=htmk(256);
    stack=0;
    result=parseitem(root,s);
    if(result)
    {
        struct strlist *list;
        HASH *t;
/*  printf("souce: %s\n",s); */
/*  printf("input:\n"); */
/*  showlist(stack); */
/*  printf("result:\n"); */
/*  showlist(result->list); */
/*  printf("xformed:\n"); */
        list=xformlist(stack,result->list);
        rmstrlist(stack);
/*  showlist(list); */
        t=mkhash(list);
        rmstrlist(list);
        pushm(result->src,result->mac,t);
        return 0;
    }
    else
        return -1;
}
