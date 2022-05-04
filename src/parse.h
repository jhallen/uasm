#ifndef _Iparse
#define _Iparse

char *parseskip(char *curline);
char *parselabel(char **ptr);
char *parsefield(char **ptr);

void asmline(char *s);
void doifeof(void);


#endif
