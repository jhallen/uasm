/* Linker
            Copyright (C) 1998 Joseph H. Allen

TELD is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version.  

TELD is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.  

You should have received a copy of the GNU General Public License
along with TELD; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "tree.h"
#include "frag.h"
#include "input.h"
#include "symtab.h"
#include "library.h"
#include "macro.h"
#include "error.h"
#include "strlist.h"
#include "ulink.h"

/* Modes & Flags */

struct source *source;
int quiet=0;		/* Set to supress messages */
char *outfile=NULL;	/* Output file name or NULL for no output file */
char *listfile=NULL;	/* List file name or NULL for no listing */

/* Modules actually included in the program */

struct module *modules;

struct module undefined= { "Undefined", "Undefined" };

unsigned nmodules=0;

/* struct symbol **sym; */	/* Array of symbols for current module */
int nsyms;		/* No. symbols in sym */
int npubs;		/* First 'npubs' symbols are publics */
int bksyms;		/* Malloc size of sym */

/* struct section **sec; */	/* Array of sections for current module */
int nsecs;		/* No. sections in sec */
int bksecs;		/* Malloc size of sec */

void listdata() { }

/* Load a module */

void loadmodule(FILE *fd,char *objname)
{
    struct module *m=malloc(sizeof(struct module));
    struct frag *last;
    m->file=objname;
    m->next=modules;
    modules=m;
    /* Process the module */
    for(;;) switch(getrec(fd))
    {
        case iMODULE: /* Module name */
            m->name=strdup(recbuf);
            break;

        case iSECTS: /* Sections */
        {
            int x=0;
            nsecs=GETNUM();		/* No. records */
            if(nsecs>bksecs)		/* Allocate space for sections */
                if(!sec) sec=malloc((bksecs=nsecs*2)*sizeof(struct section *));
                else sec=realloc(sec,(bksecs=nsecs*2)*sizeof(struct section *));
            for(x=0;recidx!=reclen;++x)	/* Record each section */
            {
                int align=GETNUM();
                int size=GETNUM();
                sec[x]=findsect(getword());

                /* Adjust alignment to largest requested amount */
                if(align>sec[x]->align)
                        sec[x]->align=align;

                /* Fix alignment */
                if(sec[x]->offset%align)
                    sec[x]->offset+=align-sec[x]->offset%align;

                /* Record next size */
                sec[x]->inc=sec[x]->offset+size;

                /* Cross reference note */
                secref(sec[x],m,sec[x]->offset);
            }
            break;
        }

        case iSYMS: /* Symbols */
        {
            int x;
            nsyms=GETNUM();		/* No. symbols */
            npubs=GETNUM();		/* No. publics */
            if(nsyms>bksyms)
            {
                if(!sym) sym=malloc((bksyms=nsyms*2)*sizeof(struct symbol *));
                else sym=realloc(sym,(bksyms=nsyms*2)*sizeof(struct symbol *));
            }
            for(x=0;recidx!=reclen;++x)	/* Record symbols */
            {
                sym[x]=findsym(symtab,getword());
                if(!sym[x]->defline) sym[x]->defline=strdup(getword());
                else getword();
                if(x>=npubs) addlinkref(sym[x],m);
            }
            break;
        }

        case iXDEFS: /* Public symbol definitions */
        {
            int x;
            for(x=0;recidx!=reclen;++x)
                if(sym[x]->v)
                    fprintf(stderr,"Link error: symbol '%s' multiply defined\n",sym[x]->name);
                else
                {
                    sym[x]->v=interm();
                    sym[x]->module=m;
                }
            break;
        }

        case iFRAG: /* Fragment */
        {
            int n=GETNUM();				/* Section no. */
            int ofst=GETNUM();				/* Offset */
            int len=reclen-recidx;			/* Size */
            last=addfrag(sec[n],sec[n]->offset+ofst,recbuf+recidx,len,sec[n]->align);	
            						/* Add the fragment */
            break;
        }

        case iFIXUPS: /* Fixups */
        {
            GETNUM();				/* No. fixups */
            while(recidx!=reclen)
            {
                int ofst=GETNUM();				/* Offset */
                int type=GETNUM();				/* Type */
                Tree *v=interm();				/* Value */
                Tree *msg=interm();				/* Message */
                fixup(last->owner,type,ofst,v,msg);		/* Append fixup to fragment */
            }
            break;
        }

        case iEND: /* End of module */
        {
            updsects();				/* Set all offsets to inc in sections */
            return;
        }
    }
}

/* Process file name */

int first;	/* Set after first object module is encountered */

void dofile(char *objname)
{
    FILE *fd=fopen(objname,"rb");
    int c;
    if(fd==NULL)
    {
        fprintf(stderr,"Link Error: Couldn\'t open file \'%s\'\n",objname);
        err=1;
    }
    else
    {
        do
            switch(c=getc(fd))
            {
                case iMODULE: /* A module */
                    ungetc(c,fd);
                    if(!first) first=1, loadmodule(fd,objname);	/* First module is root */
                    else library(fd,objname);			/* All others to library */
                    break;

                case -1: /* End of file */
                    break;

                default: /* Must be a text file */
                {
                    int bksiz, len;
                    char *name;
                    do
                    {
                        /* Name buffer */
                        name=(char *)malloc(bksiz=13);
                        /* Skip whitespace */
                        while(c==' ' || c=='\t' || c=='\r' || c=='\n')
                            c=getc(fd);
                        if(c!= -1)
                        {
                            /* Copy name to name buffer */
                            for(len=0;
                                            c!= -1 && c!=' ' && c!='\t' && c!='\r' && c!='\n';
                                            c=getc(fd))
                            {
                                if(len==bksiz-1) name=(char *)realloc(name,bksiz+=13);
                                name[len++]=c;
                            }
                            name[len]=0;
                            /* Process file name */
                            dofile(name);
                        }
                    }
                        while(c!=-1);
                }
            }
            while(c!=-1);
        fclose(fd);
    }
}

/* Position the sections */

int checkflg;

void check(void *obj,char *name,struct section *sec)
{
    if(!sec->set && sec->align) checkflg=1;
}

void listerr(void)
{
}

struct strlist *locdata;

void position(void)
{
    char buf[512];
    unsigned long addr;
    unsigned long ld;
    struct section *sec;
    int x,y;

    /* Get sections to bind from stdin */
    goto start;
    loop1:
    if(locdata)
    {
        strcpy(buf,locdata->str);
        locdata=locdata->next;
    }
    else
    {
        if(!quiet) printf(">");
        gets(buf);
    }
    x=0;
    while(buf[x]==' ' || buf[x]=='\t') ++x;
    for(y=x;buf[y] && buf[y]!=' ' && buf[y]!='\t' && buf[y]!='+' && buf[y]!=':';++y);
    sscanf(buf+x,"%lx",&addr);
    ld=addr;
    sscanf(buf+x,"%*x,%lx",&ld);
    ld-=addr;
    loop:
    for(x=y;buf[x]==' ' || buf[x]=='\t' || buf[x]==':' || buf[x]=='+';++x);
    for(y=x;buf[y] && buf[y]!=' ' && buf[y]!='\t' && buf[y]!='+' && buf[y]!=':';++y);
    if(x!=y)
    {
        char c=buf[y]; buf[y]=0;
        sec=findsect(buf+x);
        if(sec->align && !sec->set) addr=bind(sec,addr,ld);
        else if(sec->align) printf("Already did section %s\n",sec->name);
        else if(!sec->align) printf("No such section %s\n",sec->name);
        buf[y]=c;
        goto loop;
    }
    start:
    checkflg=0;
    htall(sections,check,NULL);
    if(checkflg) goto loop1;
}

/* Map file/cross-reference listing */

void linklist(char *name)
{
    FILE *f=fopen(name,"w");
    if(!f)
    {
        fprintf(stderr,"Error: could not open map file '%s'\n",name);
        return;
    }

    listsects(f);

    linkxref(f);

    fclose(f);
}

/* Main */

int main(int argc,char *argv[])
{
    int x;

    if(argc<2)
        fprintf(stderr,"\
ulink [-q] [-l file] [-o file] [-loc link[,load]:sect+sect+...]... files\n\
\n\
'-q'     Suppress messages\n\
\n\
'files'  Names of object files, libraries and text files containing\n\
         additional file names.  First module found in this list is the\n\
         root module of the program.  Each other module found here is only\n\
         included if there is a reference path from the root module to it.\n\
\n\
         A library is one or more object modules simply concatenated together.\n\
\n\
'-o'     Gives name of binary output file.  If no name is given, no output file\n\
         is generated.  Note that the first byte of the output file is always\n\
         the first generated byte, regardless of load address.\n\
\n\
'-l'     Gives name of map/cross-reference listing file.  If no name is given\n\
         no map file is generated\n\
\n\
'-loc link[,load]:sect+sect+sect...'\n\
         Locate sections:\n\
           'link' is the starting address of the given sections in hex.  This\n\
                  is the address the sections are expected to be at when the\n\
                  program is executed and is used to link symbols together.\n\
           'load' is the load address of the sections in hex.  It is the\n\
                  address in the binary output file where the sections will\n\
                  be placed by the linker.  If this is left out, it will\n\
                  default to the same as the link address.\n\
           sect+sect...\n\
                  list of sections to be located.  The first section in the\n\
                  list is located at the given address.  Additional sections\n\
                  are located directly after the first.\n\
\n\
         If not all sections are located on the command line, the linker will\n\
         prompt with '>' for the locations of the remaining sections.\n\
\n\
         Note that more than one '-loc' may be specified; there should be one\n\
         for each fixed address in the desired memory map.\n\
\n\
Universal Linker v1.0  (C) 2000 Joseph H. Allen\n\
"), exit(1);

    for(x=1;argv[x];++x)
        if(!strcmp(argv[x],"-q"))		/* Quite mode */
            quiet=1;
        else if(!strcmp(argv[x],"-loc"))	/* Locate */
            if(!argv[x+1])
                fprintf(stderr,"Link Error: Missing location data after -loc\n"),
                exit(1);
            else
            {
                struct strlist *l=malloc(sizeof(struct strlist));
                l->next=locdata;
                l->str=argv[++x];
                locdata=l;
            }
        else if(!strcmp(argv[x],"-l"))	/* Listing file name */
            if(!argv[x+1])
                fprintf(stderr,"Link Error: Missing listing file name\n"),
                exit(1);
            else if(listfile)
                fprintf(stderr,"Link Error: Listing file already specified\n"),
                exit(1);
            else
                listfile=argv[++x];
        else if(!strcmp(argv[x],"-o"))	/* Output file name */
            if(!argv[x+1])
                fprintf(stderr,"Link Error: Missing memory image output file name\n"),
                exit(1);
            else if(outfile)
                fprintf(stderr,"Link Error: Memory image file already specified\n"),
                exit(1);
            else
                outfile=argv[++x];
        else dofile(argv[x]);	/* First pass process file */

    resolve();		/* Link in necessary library object modules */

    if(!quiet)
        showsects();		/* Display the sections */

    position();		/* Position the sections */

    link(1);		/* Fixup modules, now that addresses are known */

    finddata();		/* Determine image size */

    if(listfile)
        linklist(listfile);	/* Generate listing file */

    if(outfile)
    {
        FILE *f=fopen(outfile,"wb");
        if(f) image(f);	/* Emit output file */
        else fprintf(stderr,"Couldn't open file %s\n",outfile);
    }

    return err;
}
