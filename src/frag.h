/* Sections and fragments */

#ifndef _Ifrag
#define _Ifrag 1

#include "tree.h"

#include "hash.h"


struct section
{
    char *name;			/* Section name */
    int no;			/* Section no. */
    int align;			/* Max alignment of section */
    unsigned long offset;		/* Current offset of section */
    struct frag *frags, *last;	/* Fragments composing this section */

    /* For linker */
    unsigned long inc;		/* Increment for current module */
    struct module *module;		/* Module where this was first defined */
    unsigned long addr;		/* Address assigned to this section */
    int set;			/* Set if address is assigned */
    unsigned long ldofst;		/* Load offset for section */
    unsigned long start;		/* Lowest code offset */
    unsigned long end;		/* Highest code offset */

    /* Modules contributing to this section (for listing) */
    struct module **contrib;	/* Modules contributing to section */
    unsigned *contribo;		/* Starting subsection of for Nth module */
    int ncontrib;
    int contribsz;
};

/* Items are composed of space or data fragments. A fragment can be
    * assembled data (data!=NULL) or empty space (data==NULL).  Fragments contain
    * fixup expressions.
    */

struct frag
{
    struct frag *next;
    struct section *owner;
    unsigned long align;
    unsigned long offset;
    int bksize;
    unsigned char *data;
    unsigned long len;
    struct fixup *fixups;
};

struct fixup
{
    struct fixup *next;
    int type;
    unsigned long offset;
    Tree *v;
    Tree *msg;
};

extern struct section *cursect;	/* Current section */

/* Fixup types */
#define fBYTE 1
#define fCHECK 2

void secref(struct section *sect, struct module *module, unsigned offset);

/* Emit a byte to last fragment of given section */
void catbyte(struct section *sect, unsigned char byte);

/* Emit fixup to last fragment of given section */
void fixup(struct section *sect,int type,unsigned long ofst,Tree *value,Tree *msg);

/* New fragment for current section */
void newfrag(struct section *sect,int align,int type);

struct frag *addfrag(struct section *sect,unsigned long ofst,unsigned char *buf,int len,int align);

void emitsects(char *name);
void link(int flg);

void showsects(void);

struct section *findsect(char *s);
extern struct section *cursect;
extern HASH *sections;

void image(FILE *fd);

void updsects(void);
unsigned long bind(struct section *sec,unsigned long addr,unsigned long ld);
void listsects(FILE *f);
void finddata(void);

#endif
