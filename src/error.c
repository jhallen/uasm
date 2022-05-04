/* Error flag */

#include <stdio.h>
#include <string.h>
#include "macro.h"
#include "listing.h"
#include "error.h"

int err;

/* Error printing */

char errbuf[256];	/* Error string buffer */

char *doref(struct source *src,char *s)
{
    if(src->next)
    {
        s=doref(src->next,s);
        *s++=',';
    }
    sprintf(s,"%s+%d",src->name,src->line);
    s+=strlen(s);
    return s;
}

char *srcref(char *s)
{
    *s=0;
    s=doref(source,s);
    *s++=' ';
    *s=0;
    return s;
}

void error(void)	/* Print error buffer */
{
    char tmpbuf[512];
    err=1;

    sprintf(srcref(tmpbuf),"%s",errbuf);

    listerr(tmpbuf);

    fprintf(stderr,"%s\n",tmpbuf);
}
