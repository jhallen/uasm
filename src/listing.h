#ifndef _Ilisting
#define _Ilisting 1

struct frag;

/* void listline(int lineno,char *line);
    * Close current line and start recording new line.  The source line is
    * provided here.  listerr() and listdata() append data and error messages
    * to the line.
    */
void listline(int lineno,char *s);

/* void listerror(char *s);
    * Record error message for current line.
    */
void listerr(char *s);

/* void listdata(struct frag *frag,int ofst,int len);
    * Record data generated along with current line (the line last submitted
    * with listline()).  listdata() indicates the fragment offset, the size of
    * the data and the fragment which contains the data.
    */
void listdata(struct frag *frag,int ofst,int len);

/* void listing(FILE *file);
    * Close current line and print recorded lines to listing FILE.
    */
void listing(FILE *file);

#endif
