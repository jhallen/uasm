/* Assembly listing recorder/generator */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "frag.h"
#include "strlist.h"
#include "symtab.h"
#include "error.h"
#include "listing.h"

struct listdata
{
    struct listdata *next;
    struct frag *frag;		/* Fragment data is generated in */
    int ofst;			/* Offset from start of fragment */
    int len;			/* Length of data */
};

struct listline
{
    struct listline *next;		/* Next line in linked list */
    struct strlist *errors;	/* Error messages for this line */
    int lineno;		/* Source file line no. */
    char *line;		/* Recorded source line */
    struct listdata *data;	/* Listing data */
};

struct listline *first_line, *last_line;	/* Listing line */
struct listdata *last_data;			/* Listing line data */

/* Record a listing line */

void listline(int lineno,char *s)
{
    struct listline *l=malloc(sizeof(struct listline));
    l->next=0;
    l->errors=0;
    l->line=strdup(s);
    l->data=last_data=0;
    l->lineno=lineno;
    if(last_line) last_line->next=l, last_line=l;
    else first_line=last_line=l;
    last_data=0;
}

/* Record error message for current listing line */

void listerr(char *s)
{
    if(last_line)
        last_line->errors=mkstrlist(last_line->errors,strdup(s));
}

/* Record data generated for current listing line */

void listdata(struct frag *frag,int ofst,int len)
{
    if(last_data && last_data->frag==frag && last_data->ofst+last_data->len==ofst)
        last_data->len+=len;
    else
    {
        struct listdata *l=malloc(sizeof(struct listdata));
        l->frag=frag;
        l->ofst=ofst;
        l->len=len;
        l->next=0;
        if(last_data) last_data->next=l, last_data=l;
        else last_line->data=last_data=l;
    }
}

/* Line buffer */

char *line;		/* Set if line has not yet been printed */
char linebuf[41];	/* Line buffer */
long addr;		/* Last address */
struct section *sect;	/* Last section */
int doffset;		/* Data offset */

/* Clear the line buffer */

void clrline(void)
{
    memset(linebuf,' ',40);
    linebuf[40]=0;
    sect=0;
    doffset=11;
}

/* Start new line */

void newline(int n,char *s)
{
    char bf[40];
    line=s;
    clrline();
    sprintf(bf,"%5d",n);
    memcpy(linebuf,bf,strlen(bf));
}

/* Flush line */

void flushline(FILE *file)
{
    if(line) fprintf(file,"%s%s\n",linebuf,line);
    else fprintf(file,"%s\n",linebuf);
    line=0;
    clrline();
}

/* New address/section */

void newaddr(FILE *file,struct section *newsect,long naddr)
{
    if(sect!=newsect || addr!=naddr)
    {
        char bf[40];
        if(sect) flushline(file);
        sect=newsect; addr=naddr;
        sprintf(bf,"%4.4lx",addr);
        memcpy(linebuf+6,bf,strlen(bf));
    }
}

/* Datum */

void newdata(FILE *file,int ch)
{
    char bf[40];
    if(doffset>=38)
    {
        struct section *newsect=sect;
        long naddr=addr;
        flushline(file);
        newaddr(file,newsect,naddr);
    }
    sprintf(bf,"%2.2x",ch);
    memcpy(linebuf+doffset,bf,2);
    doffset+=2;
    ++addr;
}

/* Generate listing */

void listing(FILE *file)
{
    struct listline *line;
    for(line=first_line;line;line=line->next)
    {
        struct listdata *data;
        struct strlist *l;
        for(l=line->errors;l;l=l->next)
            fprintf(file,"*************************************** %s\n",l->str);
        newline(line->lineno,line->line);
        for(data=line->data;data;data=data->next)
        {
            int x;
            for(x=0;x!=data->len;++x)
            {
                newaddr(file,data->frag->owner,data->frag->offset+data->ofst+x);
                if(data->frag->data)
                    newdata(file,data->frag->data[data->ofst+x]);
            }
        }
        flushline(file);
    }
    xreflisting(file);
}

/* Add reference to symbol */

void addref(struct symbol *sym)
{
    char tmp[512];
    srcref(tmp);
    sym->lines=mkstrlist(sym->lines,strdup(tmp));
}
