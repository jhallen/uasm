/* Assembler for experimental CPUs */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#include "frag.h"
#include "expr.h"
#include "macro.h"
#include "symtab.h"
#include "output.h"
#include "error.h"
#include "listing.h"
#include "parse.h"
#include "uasm.h"

/* Main */

int main(int argc,char *argv[])
{
    char *emitfilename=0;	/* Object file name */
    char *listfilename=0;	/* Listing file name */
    int x;
    char *s;
    char *infilename=0;
    FILE *listfile=0;

    izemit();

    if(argc==1)
        fprintf(stderr,"\
uasm [-l file] [-o file] [-I path]... file\n\
\n\
'file'	Name of assembly language source file to assemble.\n\
\n\
'-I'	Add path to include file path list.\n\
\n\
'-l'	Generate assembly listing.\n\
\n\
'-o'	Generate object file.\n\
\n\
Universal Assembler v1.0  (C) 2000 Joseph H. Allen\n\
"), exit(1);

    /* Process argument list */
    for(x=1;argv[x];++x)
        if(!strcmp(argv[x],"-o"))
            if(!argv[x+1])
                fprintf(stderr,"Error: Missing object file name\n"),
                exit(1);
            else if(emitfilename)
                fprintf(stderr,"Error: Object file name specified multiple times\n"),
                exit(1);
            else
                emitfilename=argv[++x];
        else if(!strcmp(argv[x],"-l"))
            if(!argv[x+1])
                fprintf(stderr,"Error: Missing list file name\n"),
                exit(1);
            else if(listfilename)
                fprintf(stderr,"Error: List file name specified multiple times\n"),
                exit(1);
            else
                listfilename=argv[++x];
        else if(!strcmp(argv[x],"-I"))
            if(!argv[x+1])
                fprintf(stderr,"Error: Missing include file path\n"),
                exit(1);
            else
                addpath(argv[++x]);
        else
            if(infilename)
                fprintf(stderr,"Error: only one source file allowed\n"),
                exit(1);
            else
                infilename=argv[x];

    /* Open files */
    if(!infilename)
        fprintf(stderr,"Error: source file name required\n"),
        exit(1);

    /* Use input file as source */
    pushfile(infilename,0);
    if(!source)
        exit(1);

    if(emitfilename)
        if(!(emitfile=fopen(emitfilename,"w")))
            fprintf(stderr,"Error: Couldn't open '%s'\n",emitfilename),
            exit(1);

    /* Parse the input */
    while(s=getsource()) asmline(s);
    doifeof();	/* Handle eof for .if stack */

    /* Eliminate fixups */
    link(0);

    /* Listing pass */
    if(listfilename)
        if(!(listfile=fopen(listfilename,"w")))
            fprintf(stderr,"Error: Couldn't open '%s'\n",listfilename), exit(1);
        else
        { /* Do listing */
            listing(listfile);
        }
    
    /* Write out the sections */
    if(strrchr(infilename,'.')) *strrchr(infilename,'.')=0;
    if(emitfile) emitsects(infilename);

    /* Close output files */

    if(listfile) fclose(listfile);
    if(emitfile) fclose(emitfile);

    return err;
}
