/* Simple hash table */

#ifndef _Ihash
#define _Ihash 1

#define hnext(accu,c) (((accu)<<4)+((accu)>>28)+(c))

typedef struct entry HENTRY;

struct entry
{
    char *name;		/* Symbol */
    int len;		/* Symbol length */
    HENTRY *next;		/* Next entry with same hash value */
    void *val;		/* Value bound to symbol */
};

typedef struct hash HASH;

struct hash
{
    int len;		/* No. entries in hash table - 1 */
    int llen;		/* No. entires in length table */
    HENTRY **tab;		/* Hash table itself */
    char *ltab;		/* Length table to use for find */
    char *btab;		/* Length table created specifically for this level */
    HASH *next;		/* Hash table with next broader scope */
};

unsigned long hash(char *s);
unsigned long hashn(char *s, int n);
HASH *htmk(int len);
void htrm(HASH *ht);
void *htfind(HASH *ht,char *name);
void *htfindn(HASH *ht,int hval,char *name,int len);
void *htlfind(HASH *ht,char **ptr);
void *htlfindx(HASH *ht,char **ptr,void *(*func)(void *, char *));
void *htadd(HASH *ht,char *name,void *val);
void htall(HASH *ht,void (*func)(void *obj, char *name, void *val),void *obj);
HASH *htpsh(HASH *old);
void htpop(HASH **ptr,HASH *to);

#endif
