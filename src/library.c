/* Library management */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#include "input.h"
#include "ulink.h"
#include "symtab.h"
#include "error.h"

HASH *lib;			/* Library */

struct lentry
{
    char *name;			/* Symbol name */
    unsigned long pos;		/* File offset to module */
    char *file;			/* Name of file containing module */
};

/* Load module as library */

void library(FILE *fd,char *name)
{
    unsigned long pos;

    if(!lib) lib=htmk(1024);

    pos=ftell(fd);

    for(;;)
    {
        switch(getrec(fd))
        {
            case iSYMS: /* Record names defined in this module */
            {
                int x;
                int n;
                GETNUM();			/* No. records */
                n=GETNUM();			/* No. publics */
                for(x=0;x!=n;++x)		/* Add each public to library table */
                {
                    struct lentry *f;
                    struct lentry *e=malloc(sizeof(struct lentry));
                    e->file=name;
                    e->pos=pos;
                    e->name=strdup(getword());
                    if(f=htfind(lib,e->name))
                        fprintf(stderr,
                            "Multiply defined symbol '%s' in %s and %s\n",e->name, name,f->name);
                    else
                        htadd(lib,e->name,e);
                }
                break;
            }

            case iEND:	/* All done */
                return;

            default:	/* Just skip all other types of records */
                break;
        }
    }
}

/* Find symbol in library and if found, load module associated with it */

void getlib(char *name)
{
    struct lentry *e;
    if(!lib) return;
    e=htfind(lib,name);
    if(e)
    {
        FILE *fd=fopen(e->file,"rb");
        fseek(fd,e->pos,0);
        loadmodule(fd,e->file);
        fclose(fd);
    }
}

/* Try to eliminate undefined symbols by looking in library modules */

int resflg=0;

void resolver(void *obj,char *name,void *v)
{
    struct symbol *sy = (struct symbol *)v;
    if(!sy->module && !sy->v)
    {
        getlib(sy->name);
        if(!sy->v)
        { /* Symbol is still undefined */
            fprintf(stderr,"%sUndefined symbol '%s'\n",sy->defline,sy->name);
            err=1;
            sy->module= &undefined;	/* Avoid infinite loop */
            resflg=1;
        }
    }
}

void resolve(void)
{
    if(symtab->symtab)
        do resflg=0, htall(symtab->symtab,resolver,NULL);
            while(resflg);
}
