/* Sections & fragments */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "output.h"
#include "error.h"
#include "ulink.h"
#include "listing.h"
#include "symtab.h"
#include "frag.h"

/* Section table */

HASH *sections;			/* Sections hash table */

struct section *cursect;	/* Current section */

/* Search for a section */

struct section *findsect(char *s)
{
    struct section *section;
    if(!sections) sections=htmk(16);		/* Create hash table */
    if(!(section=htfind(sections,s)))		/* Lookup section */
    { /* Not found... create new section */
        section=calloc(sizeof(struct section),1);
        section->name=strdup(s);
        section->align=1;
        section->offset=0;
        section->start=0xFFFFFFFF;
        section->end=0;
        section->ncontrib=0;
        section->contribsz=8;
        section->contrib=malloc(section->contribsz*sizeof(struct module *));
        section->contribo=malloc(section->contribsz*sizeof(unsigned));
        newfrag(section,1,1);
        htadd(sections,section->name,section);
    }
    return section;
}

void secref(struct section *sect, struct module *module, unsigned offset)
{
    if(sect->ncontrib==sect->contribsz)
    {
        sect->contribsz*=2;
        sect->contrib=realloc(sect->contrib,sizeof(struct module *)*sect->contribsz);
        sect->contribo=realloc(sect->contribo,sizeof(unsigned)*sect->contribsz);
    }
    sect->contrib[sect->ncontrib]=module;
    sect->contribo[sect->ncontrib]=offset;
    ++sect->ncontrib;
}

/* Create new fragment */

void newfrag(struct section *sect,int align,int type)
{
    struct frag *frag=malloc(sizeof(struct frag));
    frag->next=0;
    if(sect->last) sect->last->next=frag;
    else sect->frags=frag;
    sect->last=frag;
    frag->owner=sect;
    frag->align=align;
    frag->offset=sect->offset;
    if(frag->offset%frag->align!=0)
    {
        frag->offset+=frag->align-(frag->offset%frag->align);
        sect->offset=frag->offset;
    }
    frag->bksize=128;
    if(type)
        frag->data=malloc(frag->bksize);
    else
        frag->data=0, frag->bksize=0;
    frag->len=0;
    frag->fixups=0;
}

struct frag *addfrag(struct section *sect,unsigned long ofst,unsigned char *buf,int len,int align)
{
    struct frag *frag=malloc(sizeof(struct frag));
    frag->next=0;
    if(sect->last) sect->last->next=frag;
    else sect->frags=frag;
    sect->last=frag;
    frag->owner=sect;
    frag->align=align;
    frag->offset=ofst;
    if(frag->offset%frag->align)
    {
        frag->offset+=frag->align-(frag->offset%frag->align);
        sect->offset=frag->offset;
    }
    frag->bksize=frag->len=len;
    frag->data=malloc(frag->bksize);
    memcpy(frag->data,buf,len);
    frag->fixups=0;
    return frag;
}

/* Allocate space in last fragment of section */

unsigned char *rsv(struct section *sect, int amnt)
{
    listdata(sect->last,sect->last->len,amnt);
    sect->offset+=amnt;
    sect->last->len+=amnt;
    if(sect->last->len>sect->last->bksize)
        sect->last->data=realloc(sect->last->data,sect->last->bksize*=2);
    return sect->last->data+sect->last->len-amnt;
}

/* Append byte to last fragment of section */

void catbyte(struct section *sect, unsigned char byte)
{
    unsigned char *addr=rsv(sect,1);
    *addr=byte;
}

/* Append fixup to section */

void fixup(struct section *sect,int type,unsigned long ofst,Tree *value,Tree *msg)
{
    struct fixup *f=malloc(sizeof(struct fixup));
    f->type=type;
    f->offset=ofst;
    f->v=value;
    f->msg=msg;
    if(!f->msg) f->msg=mk(Stringid,"",1);
    f->next=sect->last->fixups;
    sect->last->fixups=f;
}

/* Set section offset to section inc */

void updsect(void *obj,char *name,void *v)
{
    struct section *sec = (struct section *)v;
    sec->offset=sec->inc;
}

void updsects(void)
{
    htall(sections,updsect,NULL);
}

/* Emit fragments */

char *ftypes[]=
{
    "Huh?",
    "Byte",
    "Word"
};

void emitfrag(struct frag *frag)
{
    if(frag->data)
    {
        struct fixup *f;
        int n;

        /* Emit the fragment */
        objrec(iFRAG);		/* iFRAG record */
        emitnum(frag->owner->no);	/* Section no. */
        emitnum(frag->offset);	/* Offset within section */
        emitblk(frag->data,frag->len);/* Data */
        endrec();

        /* Emit fixups for this frag */
        objrec(iFIXUPS);		/* iFIXUPS record */
        for(f=frag->fixups,n=0;f;f=f->next,++n);
        emitnum(n);			/* No. fixups */
        for(f=frag->fixups;f;f=f->next)
        {
            emitnum(f->offset);	/* Offset to datum */
            emitnum(f->type);	/* Datum type */
            emit(f->v);		/* Value */
            emit(f->msg);	/* Message */
        }
        endrec();
    }
}

/* Emit fragments in a section */

void emitsect(void *obj,char *name,void *v)
{
    struct section *sect = (struct section *)v;
    struct frag *frag;
    for(frag=sect->frags;frag;frag=frag->next) emitfrag(frag);
}

/* Emit section headers */

int nsects;

void countsect(void *obj,char *name,void *v)
{
    struct section *sect = (struct section *)v;
    sect->no=nsects++;
}

void hdrsect(void *obj,char *name,void *v)
{
    struct section *sect = (struct section *)v;
    emitnum(sect->align);		/* Alignment */
    emitnum(sect->offset);		/* Size */
    emits(sect->name);		/* Name */
}

void emithdrs(void)
{
    struct section *sect;
    nsects=0;
    objrec(iSECTS);
    if(sections) htall(sections,countsect,NULL);
    emitnum(nsects);	/* No. of sections */
    if(sections) htall(sections,hdrsect,NULL);
    endrec();
}

/* Emit output file */

void emitsects(char *name)
{
    unsigned len=strlen(name);
    int n;

    /* Module name record */
    objrec(iMODULE);
    emitblk(name,len+1);
    endrec();

    /* Emit section headers */
    emithdrs();

    /* Emit symbol table */
    emitsymtab();

    /* Emit data fragments */
    if(sections) htall(sections,emitsect,NULL);

    /* End of module record */
    objrec(iEND);
    endrec();
}

/* Link a fragment */

int linkflg;

void linkfrag(struct frag *frag)
{
    struct fixup *f, *next;
    struct fixup *new=0;
    for(f=frag->fixups;f;f=next)
    {
        next=f->next;
        f->v=simp(f->v);
        if(f->v->Int.id==Intid)
        {
            switch(f->type)
            { /* Need to check ranges here */
                case fCHECK:
                    if(f->v->Int.val)
                        error0(f->msg->String.val);
                    break;
                case fBYTE:
                    frag->data[f->offset]=f->v->Int.val;
                    break;
            }
            rm(f->v);
            free(f);
        }
        else
        {
            if(linkflg)
                error1("%s: could not simplify fixup expression",f->msg->String.val);
            f->next=new, new=f;
        }
    }
    frag->fixups=new;
}

/* Link a section */

void linksect(void *obj,char *name,void *v)
{
    struct section *sect = (struct section *)v;
    struct frag *frag;
    for(frag=sect->frags;frag;frag=frag->next) linkfrag(frag);
}

/* Link all sections */

void link(int flg)
{
    linkflg=flg;
    if(sections) htall(sections,linksect,NULL);
}

/* Show the sections so the user can locate them */

FILE *showfile;

void showsect(void *obj,char *name, void *v)
{
    struct section *sect = (struct section *)v;
    fprintf(showfile," %s",name);
}

void showsects(void)
{
    if(!showfile) showfile=stderr;
    fprintf(showfile,"Sections:");
    if(sections) htall(sections,showsect,NULL);
    fprintf(showfile,"\n");
}

/* Bind section to an address */

unsigned long bind(struct section *sec,unsigned long addr,unsigned long ld)
{
    struct frag *frag;
    sec->addr=addr;
    sec->ldofst=ld;
    sec->set=1;
    return addr+sec->offset;
}

/* Generate image file */

unsigned long low;
unsigned long high;
unsigned char *mem;

void lowhi(void *obj,char *name,void *v)
{
    struct section *sec = (struct section *)v;
    struct frag *frag;
    for(frag=sec->frags;frag;frag=frag->next)
    {
        if(frag->offset<sec->start) sec->start=frag->offset;
        if(frag->offset+frag->len>sec->end) sec->end=frag->offset+frag->len;
        if(sec->addr+frag->offset<low)
            low=sec->addr;
        if(sec->addr+frag->offset+frag->len>high)
            high=sec->addr+frag->offset+frag->len;
    }
    if(sec->addr<low) low=sec->addr;
    if(sec->addr+sec->offset>high) high=sec->addr+sec->offset;
}

void tomem(void *obj,char *name,void *v)
{
    struct section *sec = (struct section *)v;
    struct frag *frag;
    for(frag=sec->frags;frag;frag=frag->next)
        if(frag->len)
            memcpy(mem+sec->addr+frag->offset-low,frag->data,frag->len);
}

void finddata(void)
{
    low=0xFFFFFFFF;
    high=0;
    htall(sections,lowhi,NULL);
}

void image(FILE *fd)
{
    if(high>low)
    {
        mem=calloc(1,high-low);
        htall(sections,tomem,NULL);
        fwrite(mem,1,high-low,fd);
    }
}

/* Generate listing */

void dolistsect(void *obj,char *name,void *v)
{
    FILE *f = (FILE *)obj;
    struct section *sect = (struct section *)v;
    int x;
    fprintf(f,"%-20s %8lx %8lx",sect->name,sect->addr,sect->offset);
    if(sect->end>sect->start)
        fprintf(f," %8lx - %-8lx\n",sect->start,sect->end-1);
    else
        fprintf(f,"\n");
    
    for(x=0;x!=sect->ncontrib;++x)
        fprintf(f," %-19s %8lx\n",sect->contrib[x]->name,sect->contribo[x]+sect->addr);
}

void listsects(FILE *f)
{
    fprintf(f,"Memory map\n");
    fprintf(f,"----------\n\n");
    fprintf(f,"Section               Address     Size   Data at offsets\n");
    fprintf(f,"-------               ------- -------- -------------------\n");
    htall(sections,dolistsect,f);
}
