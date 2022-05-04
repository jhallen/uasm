#ifndef _Ilink
#define _Ilink 1

struct module
{
    char *file;            /* File containing object module */
    char *name;            /* Name of object module */
    struct module *next;
};

extern struct symbol **sym;	/* Array of symbols for current module */
extern struct section **sec;	/* Array of sections for current module */

/* Load a module */
void loadmodule(FILE *fd,char *objname);
extern struct module undefined;

#endif
