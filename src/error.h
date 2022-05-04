/* Error printers */

#ifndef _Ierror
#define _Ierror 1

extern struct source *source;	/* Current source file */
char *srcref(char *);
extern int err;			/* Error flag */
extern char errbuf[];		/* Error string buffer */
void error(void);		/* Error message handler */
#define error0(s) ( sprintf(errbuf,(s)), error() )
#define error1(s,a) ( sprintf(errbuf,(s),(a)), error() )
#define error2(s,a,b) ( sprintf(errbuf,(s),(a),(b)), error() )

#endif
