/* Symbol table */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#include "uasm.h"
#include "error.h"
#include "strlist.h"
#include "frag.h"
#include "ulink.h"
#include "symtab.h"

struct symtab symtab[1];

/* Search for symbol */

unsigned nlabels;				/* No. of symbols */

struct symbol *findsym(struct symtab *symtab, char *s)
{
    struct symbol *symbol;
    if(!symtab->symtab) symtab->symtab=htmk(1024);		/* Create hash table */
    if(!(symbol=htfind(symtab->symtab,s)))			/* Lookup symbol */
    { /* Not found... create new symbol */
        symbol=malloc(sizeof(struct symbol));
        symbol->name=strdup(s);
        symbol->state=0;
        symbol->no= -1;
        symbol->pub=0;
        symbol->v=0;
        symbol->defline=0;
        symbol->lines=0;
        /* Linker stuff */
        symbol->ref=malloc((symbol->bksize=10)*sizeof(struct module *));
        symbol->nref=0;
        symbol->module=0;
        htadd(symtab->symtab,symbol->name,symbol);
        ++nlabels;
    }
    return symbol;
}

/* Add a module reference if there was not one already */

void addlinkref(struct symbol *symbol, struct module *module)
{
    if(symbol->nref==symbol->bksize)
        symbol->ref=realloc(symbol->ref,(symbol->bksize*=2)*sizeof(struct module *));
    symbol->ref[symbol->nref++]=module;
}

/* Emit public symbols */

static int npubs;
static int nrefs;

static void countsym(void *obj,char *name,struct symbol *sym)
{
    if(sym->pub) sym->no=npubs++;
    if(!sym->pub && !sym->v) sym->no=nrefs++;
}

static void emitpub(void *obj,char *name,struct symbol *sym)
{
    if(sym->pub)
    {
        emits(name);
        if(sym->defline) emits(sym->defline);
        else if(sym->lines) emits(sym->lines->str);
        else emits("");
    }
}

static void emitref(void *obj,char *name,struct symbol *sym)
{
    if(!sym->pub && !sym->v)
    {
        sym->no+=npubs, emits(name);
        if(sym->defline) emits(sym->defline);
        else if(sym->lines) emits(sym->lines->str);
        else emits("");
    }
}

/* Emit symbol names.  Assign symbol no.s to each symbol */

static void emitsyms(void)
{
    npubs=0;
    nrefs=0;
    if(symtab->symtab) htall(symtab->symtab,countsym,NULL);
    objrec(iSYMS);
    emitnum(nrefs+npubs);
    emitnum(npubs);
    if(symtab->symtab) htall(symtab->symtab,emitpub,NULL);
    if(symtab->symtab) htall(symtab->symtab,emitref,NULL);
    endrec();
}

/* Emit public symbol values */

static void emitdef(void *obj,char *name,struct symbol *sym)
{
    if(sym->pub)
        if(sym->v)
            emit(sym->v);
        else
            error1("Undefined public '%s'",name),
            emitc(0); /* This is a bug... */
}

static void emitdefs(void)
{
    objrec(iXDEFS);
    if(symtab->symtab) htall(symtab->symtab,emitdef,NULL);
    endrec();
}

/* Emit symbol no. */

void emitsymbol(struct symbol *sym)
{
    if(sym->no== -1)
    {
        fprintf(stderr,"Huh?  Emit non-external symbol %s?\n",sym->name);
        exit(1);
    }
    else
        emitnum(sym->no);
}

/* Emit symbol table */

void emitsymtab(void)
{
    emitsyms();
    emitdefs();
}

/* Print cross-reference listing */

struct symbol **symarray;
int symnth;

void buildarray(void *obj,char *name,struct symbol *sym)
{
    symarray[symnth++]=sym;
}

int symcmp(struct symbol **a,struct symbol **b)
{
    return strcmp((*a)->name,(*b)->name);
}

void xreflisting(FILE *file)
{
    fprintf(file,"\n** Symbol cross reference listing **\n\n");
    if(symtab->symtab && nlabels)
    {
        int n;
        symarray=malloc(nlabels*sizeof(struct symbol *));
        symnth=0;
        htall(symtab->symtab,buildarray,NULL);
        qsort(symarray,symnth,sizeof(struct symbol *),symcmp);
        for(n=0;n!=symnth;++n)
        {
            struct strlist *str;
            fprintf(file,"%s ",symarray[n]->name);
            if(symarray[n]->v)
            {
                fprintf(file,"( %s) =",symarray[n]->defline);
                show(symarray[n]->v,file,0,0);
                fprintf(file,"\n");
            }
            else
                fprintf(file,"\n");
            fprintf(file,"  ");
            for(str=symarray[n]->lines;str;str=str->next)
                fprintf(file,"%s",str->str);
            fprintf(file,"\n\n");
        }
    }
}

void linkxref(FILE *file)
{
    fprintf(file,"\nPublic symbols cross reference listing\n");
    fprintf(file,"--------------------------------------\n\n");
    if(symtab->symtab && nlabels)
    {
        int n;
        symarray=malloc(nlabels*sizeof(struct symbol *));
        symnth=0;
        htall(symtab->symtab,buildarray,NULL);
        qsort(symarray,symnth,sizeof(struct symbol *),symcmp);
        for(n=0;n!=symnth;++n)
        {
            struct strlist *str;
            int j;
            fprintf(file,"%s ",symarray[n]->name);
            if(symarray[n]->v)
            {
                fprintf(file,"(%s) =",symarray[n]->module->name);
                show(symarray[n]->v,file,0,0);
                fprintf(file,"\n");
            }
            else
                fprintf(file,"\n");
            for(j=0;j!=symarray[n]->nref;++j)
                fprintf(file," %s",symarray[n]->ref[j]->name);
            fprintf(file,"\n\n");
        }
    }
}
