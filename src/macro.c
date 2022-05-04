/* Input reader & macro expansion */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uasm.h"
#include "tree.h"
#include "expr.h"
#include "error.h"
#include "parse.h"
#include "syntax.h"
#include "symtab.h"
#include "strlist.h"
#include "listing.h"
#include "macro.h"

/* Input source stack - lines of input come from a stack of files and
    * macros.  Macro expansions and .include directives cause a source to
    * be pushed onto this stack.
    */

int gline;		/* Global line no. */

/* Input source stack */
/* The input could come from the main file, an include file or a macro. */

struct source *source;	/* Stack of input sources */

/* Get next line from source stack */

char inbuf[LINESZ];	/* Next input line buffer */

char *getsource(void)
{
    char *rtn=0;
    while(!rtn && source)
        if(source->file)
            /* Top of stack is a file */
            if(fgets(inbuf,LINESZ-1,source->file)) 	/* Read file line */
            { /* Success! */
                int len=strlen(inbuf);
                /* Eliminate terminating newline */
                if(len && inbuf[len-1]=='\n') inbuf[len-1]=0;
                rtn=inbuf;
                ++source->line;
                ++gline;
                listline(source->line,rtn);			/* Record line for listing */
            }
            else
            { /* File exhausted.  Close it and try next source on stack */
                struct source *next=source->next;
                fclose(source->file);
                free(source);
                source=next;
            }
        else
            /* Top of source stack is a macro */
            if(source->mac) /* Any more macro lines? */
            { /* Perform macro substitution on macro line */
                int x, y, n;
                struct strlist *mac=source->mac;
                ++source->line;
                ++gline;
                if(source->args) substcopy(inbuf,mac->str,source->args);
                else strcpy(inbuf,mac->str);
                source->mac=mac->next;
                rtn=inbuf;
            }
            else
            { /* Close input macro.  Try next source on stack. */
                struct source *next=source->next;
                rmhash(source->args);
                free(source);
                source=next;
            }
    return rtn;
}

/* Include file search path */

struct strlist *incs;

/* Add an include file search path */

void addpath(char *s)
{
    struct strlist *inc=malloc(sizeof(struct strlist));
    inc->next=incs;
    inc->str=strdup(s);
    incs=inc;
}

/* Push a file as the input source */

void pushfile(char *s,int flg)
{
    FILE *f;
    char buf[1024];
    struct source *src;

    f=fopen(s,"r");
    if(!f)
    {
        /* Scan include file search paths if flag set */
        if(flg)
        {
            struct strlist *inc;
            for(inc=incs;inc;inc=inc->next)
            {
                sprintf(buf,"%s/%s",inc->str,s);
                if(f=fopen(buf,"r")) break;
            }
        }
        if(!f)
        {
            error1("Couldn't open file \'%s\'",s);
            return;
        }
    }
    src=malloc(sizeof(struct source));
    src->name=strdup(s);
    src->file=f;
    src->line=0;
    src->next=source;
    source=src;
}

/* Push string list as input source */

void pushm(char *name,struct strlist *body,HASH *args)
{
    struct source *src;
    src=malloc(sizeof(struct source));
    src->line=0;
    src->name=name;
    src->file=0;
    src->mac=body;
    src->args=args;
    src->next=source;
    source=src;
}

/* Push a macro as the input source */

void pushmacro(struct macro *mac,char *s)
{
    HASH *h=htmk(64);
    char *arg;
    struct strlist *str;
    /* Match-up args */
    for(arg=parsefield(&s),str=mac->args;str && arg;
                    str=str->next,arg=parsefield(&s))
    {
        struct subst *subst=malloc(sizeof(struct subst));
        subst->name=strdup(str->str);
        subst->subst=arg;
        htadd(h,subst->name,subst);
        if(*s==',') ++s;
    }
    if(str || arg || *s && *s!=';')
    {
        error0("Incorrect no. of macro args");
        return;
    }
    pushm(mac->name,mac->body,h);
}

/* Push a foreach as the input source */

void pushforeach(struct macro *mac,char *s)
{
    struct strlist *list, *n;

    list=parselist(&s);
    if(!list)
    {
        error0("Missing argument");
        return;
    }

    list=reverselist(list);

    /* Now push macro for each argument */
    while(list)
    {
        Tree *v=0;
        char *sp=list->str;
        n=list->next;

        if(list->str[0]=='"' && (v=expr(symtab,&sp))->String.id==Stringid)
        {
            int n;
            int x;
            sp=v->String.val;
            n=v->String.len;
            for(x=n;x!=0;)
            {
                char bf[15];
                HASH *h=htmk(8);
                struct subst *subst=malloc(sizeof(struct subst));
                --x;
                sprintf(bf,"%d",sp[x]);
                subst->name=strdup(mac->args->str);
                subst->subst=strdup(bf);
                htadd(h,subst->name,subst);
                pushm(mac->name,mac->body,h);
            }
        }
        else
        {
            HASH *h=htmk(8);
            struct subst *subst=malloc(sizeof(struct subst));
            subst->name=strdup(mac->args->str);
            subst->subst=list->str;
            htadd(h,subst->name,subst);
            pushm(mac->name,mac->body,h);
        }

        if(v) rm(v);

        free(list);
        list=n;
    }
}

void doinclude(struct macro *macro,char *s)
{
    Tree *v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0("Missing file name");
        return;
    }
    if(*s && *s!=';') error0("Extra junk after file name");
    if(v->String.id!=Stringid) error0("Include file name must be a string");
    else pushfile(v->String.val,1);
    rm(v);
}

HASH *macros;	/* Hash table of macros */

/* Record input until .end */

struct strlist *record(void)
{
    int count;
    char *s;
    struct strlist *first, *last;

    /* Record whole lines until we see a .endm */
    count=1;
    first=0;
    while(s=getsource())
    {
        char *label;
        char *mnem;
        char *dupped=strdup(s);
        label=parselabel(&s);
        mnem=parsefield(&s);
        if(mnem && !strcmp(mnem,".end") && !--count)
        {
            free(dupped);
            free(mnem);
            if(label) free(label);
            break;
        }
        else
        {
            struct strlist *arg;
            if(mnem) if(!strcmp(mnem,".macro") || !strcmp(mnem,".if") ||
                                                            !strcmp(mnem,".foreach") || !strcmp(mnem,".insn")) ++count;
            arg=mkstrlist(NULL,dupped);
            if(!first) first=last=arg;
            else last->next=arg, last=arg;
        }
        if(label) free(label);
        if(mnem) free(mnem);
    }
    if(!s) error0("Input exhausted before .end");
    return first;
}

/* Get argument list */

struct strlist *args(char **ptr)
{
    struct strlist *first, *last;
    char *s= *ptr;
    /* Record arg names */
    first=0;
    while(*s==',')
    {
        char *a;
        ++s;
        a=parsefield(&s);
        if(!a)
            error0("missing arg after ,");
        else
        {
            struct strlist *arg=mkstrlist(NULL,a);
            if(!first) first=last=arg;
            else last->next=arg, last=arg;
        }
    }
    *ptr =s;
    return first;
}

/* Record macro */

void domacro(struct macro *macro,char *s)
{
    struct macro *m;
    char *name=parsefield(&s);	/* Get macro's name */

    /* Create macro */
    m=malloc(sizeof(struct macro));
    m->name=name;
    m->func=pushmacro;
    m->args=args(&s);	/* Get args */
    m->body=record();	/* Get body */

    if(!name)
        error0("Macro name required");
    else if(findmacro(name))
        error1("Macro '%s' already exists",name);
    else htadd(macros,m->name,m);
}

/* Record foreach */

void doforeach(struct macro *macro,char *s)
{
    struct macro *m;
    struct strlist *arg, *list;
    char *name=parsefield(&s);	/* Get macro's name */

    /* Create macro */
    m=malloc(sizeof(struct macro));
    m->name=name;
    m->func=pushforeach;
    m->args=args(&s);
    m->body=record();

    if(!name)
        error0("Macro name required");
    else if(findmacro(name))
        error1("Macro '%s' already exists",name);
    else if(!m->args || m->args->next)
        error0("Foreach must have one and only one arg");
    else htadd(macros,m->name,m);
}

/* Assembler directives/macros table */

/* Built-in directives */

struct macro macrotab[]=
{
    {	".include"	,doinclude	},
    {	".macro"	,domacro	},
    {	".foreach"	,doforeach	},
    {	".if"		,doif		},
    {	".elseif"	,doelseif	},
    {	".else"		,doelse		},
    {	".end"		,doend		},
    {	".sect"		,dosect		},
    {	".emit"		,doemit		},
    {	".equ"		,doequ		},
    {	".set"		,doset		},
    {	".public"	,dopublic	},
    {	".space"	,dospace	},
    {	".align"	,doalign	},
    {	".rule"		,dorule		},
    {	".insn"		,doinst		},
    {	".showsyntax"	,doshowsyntax	},
    {	".errif"	,doerrif	},
    {	".small"	,dosmall	},
    {	".bigger"	,dobigger	},
    {	".biggest"	,dobiggest	},
    {	".done"		,dodone		},
    {	0	}
};

/* Look up a macro */

struct macro *findmacro(char *s)
{
    if(!macros)
    {
        int x;
        macros=htmk(512);
        for(x=0;macrotab[x].name;++x)
            htadd(macros,macrotab[x].name,macrotab+x);
    }
    return htfind(macros,s);
}
