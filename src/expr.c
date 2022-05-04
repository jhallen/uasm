/* Expression parser */

#include <stdio.h>
#include "hash.h"
#include "symtab.h"
#include "frag.h"
#include "uasm.h"
#include "error.h"
#include "parse.h"
#include "expr.h"

extern void *malloc();
extern void *realloc();
extern void free();

static char *ptr="";		/* Input pointer */
static int col=0;		/* Column no. 'ptr' is at.  0 is first */
static int lvl= -1;		/* Indentation level of current line */

/* Skip over whitespace */

static void skipws()
{
    while(1) switch(*ptr)
    {
        case ' ':
            ++ptr; ++col;
            break;

        case '\t':
            ++ptr; col+=8-col%8;
            break;

        case  ';':
        case '\r':
        case '\n':
        case 0:
            return;
                                                
        default:
            if(lvl== -1) lvl=col;
            return;
    }
}

/* Get a character which might be an escape sequence */

static int escape()
{
    if(*ptr=='\\')
    {
        ++ptr; ++col;
        switch(*ptr)
        {
        case '^':
            ++ptr; ++col;
            if(*ptr>='@' && *ptr<='_') return ++col, *ptr++-'@';
            else if(*ptr=='?') return ++col, ++ptr, 127;
            else return escape();

        case '8': case '9':
            return ++col, *ptr++-'0';

        case 'x': case 'X':
        {
            int num=0;
            ++col; ++ptr;
            if(*ptr>='0' && *ptr<='9') num=(++col, *ptr++-'0');
            else if(*ptr>='a' && *ptr<='f') num=(++col, *ptr++-'a'+10);
            else if(*ptr>='A' && *ptr<='F') num=(++col, *ptr++-'A'+10);
            if(*ptr>='0' && *ptr<='9') num=num*16+(++col, *ptr++-'0');
            else if(*ptr>='a' && *ptr<='f') num=num*16+(++col, *ptr++-'a'+10);
            else if(*ptr>='A' && *ptr<='F') num=num*16+(++col, *ptr++-'A'+10);
            return num;
        }

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            {
                int num= *ptr++-'0'; ++col;
                if(*ptr>='0' && *ptr<='7')
                    num=num*8+*ptr++-'0', ++col;
                if(*ptr>='0' && *ptr<='7')
                    num=num*8+*ptr++-'0', ++col;
                return num;
            }

        case 'a': return ++ptr, ++col, 7;
        case 'b': return ++ptr, ++col, 8;
        case 'e': return ++ptr, ++col, 27;
        case 'f': return ++ptr, ++col, 12;
        case 'n': return ++ptr, ++col, 10;
        case 'r': return ++ptr, ++col, 13;
        case 't': return ++ptr, ++col, 9;
        
        case '\\': return ++col, *ptr++;

            default: return escape();
        }
    }
    else if(*ptr!='\t') return ++col, *ptr++;
    else return col+=8-col%8, *ptr++;
}

/* Get a word */

static char *wrdbuf;
static int wrdsiz;

static char *word()
{
    skipws();
    if(*ptr>='a' && *ptr<='z' || *ptr>='A' && *ptr<='Z' || *ptr=='_')
    {
        char *s=ptr;
        char *t;
        while(isalnum(*ptr) || *ptr=='_') ++ptr, ++col;
        if(ptr-s+1>wrdsiz)
            if(wrdbuf) wrdbuf=realloc(wrdbuf,wrdsiz*=2);
            else wrdbuf=malloc(wrdsiz=64);
        if(ptr-s) memcpy(wrdbuf,s,ptr-s);
        wrdbuf[ptr-s]=0;
        return wrdbuf;
    }
    else return 0;
}

/* Operator table */

extern OPR oCALL, oSUB;

/* Method values
    *  0 means normal infix, prefix or postfix operator
    *  2 is for the ?: operator
    *  5 means look for a terminating )
    */

/*		  Name	 Prefix	 Infix	 Postfix Prec.	 Assoc.	 Method	 Func */

OPR oPAREN	={"("	,&oPAREN,&oCALL	,0	,150	,0	,5	,NULL		};

OPR oCALL	={"("	,&oPAREN,&oCALL	,0	,130	,0	,5	,Parenid	};

OPR oCOM	={"~"	,&oCOM	,0	,0	,120	,0	,0	,Comid		};
OPR oNEG	={"neg"	,&oNEG	,&oSUB	,0	,120	,0	,0	,Negid		};
OPR oNOT	={"!"	,&oNOT	,0	,0	,120	,0	,0	,Notid		};

OPR oSHR	={">>"	,0	,&oSHR	,0	,110	,0	,0	,Shrid		};
OPR oSHL	={"<<"	,0	,&oSHL	,0	,110	,0	,0	,Shlid		};

OPR oMUL	={"*"	,0	,&oMUL	,0	,100	,0	,0	,Mulid		};
OPR oDIV	={"/"	,0	,&oDIV	,0	,100	,0	,0	,Divid		};
OPR oMOD	={"%"	,0	,&oMOD	,0	,100	,0	,0	,Modid		};
OPR oAND	={"&"	,0	,&oAND	,0	,100	,0	,0	,Andid		};

OPR oADD	={"+"	,0	,&oADD	,0	,90	,0	,0	,Addid		};
OPR oSUB	={"-"	,&oNEG	,&oSUB	,0	,90	,0	,0	,Subid		};
OPR oOR		={"|"	,0	,&oOR	,0	,90	,0	,0	,Orid		};
OPR oXOR	={"^"	,0	,&oXOR	,0	,90	,0	,0	,Xorid		};

OPR oEQ		={"=="	,0	,&oEQ	,0	,80	,0	,0	,Eqid		};
OPR oNE		={"!="	,0	,&oNE	,0	,80	,0	,0	,Neid		};
OPR oLT		={"<"	,0	,&oLT	,0	,80	,0	,0	,Ltid		};
OPR oGT		={">"	,0	,&oGT	,0	,80	,0	,0	,Gtid		};
OPR oLE		={"<="	,0	,&oLE	,0	,80	,0	,0	,Leid		};
OPR oGE		={">="	,0	,&oGE	,0	,80	,0	,0	,Geid		};

OPR oLAND	={"&&"	,0	,&oLAND	,0	,70	,0	,0	,Landid		};

OPR oLOR	={"||"	,0	,&oLOR	,0	,60	,0	,0	,Lorid		};

OPR oMUX	={"?"	,0	,&oMUX	,0	,50	,1	,2	,Muxid		};

OPR oCOMMA	={","	,0	,&oCOMMA,0	,20	,0	,0	,NULL		};

/* Operator scanner */

/* Initial operators */

static OPR *ioprtab[]=
{ &oCALL, &oCOM, &oNOT, &oSHR, &oSHL, &oMUL, &oDIV, &oMOD, &oAND, &oADD,
            &oSUB, &oOR, &oXOR, &oEQ, &oNE, &oLT, &oGT, &oLE, &oGE, &oLAND, &oLOR,
            &oMUX, 0 };

static HASH *oprtab;

/* Initialize operator table */

static void izoprtab()
{
    int x;
    oprtab=htmk(64);
    for(x=0;ioprtab[x];++x) htadd(oprtab,ioprtab[x]->name,ioprtab[x]);
}

/* Look up an operator */

static OPR *opr()
{
    char *optr=ptr;
    OPR *o;
    if(!oprtab) izoprtab();
    o=htlfind(oprtab,&ptr);
    col+=ptr-optr;
    return o;
}

/* Parse an expression.  Returns parse-tree or NULL if there was an error.
    */

static char *strbuf;		/* String buffer */
static int bufsiz;

static Tree *parseexpr(symtab,prec)
struct symtab *symtab;
{
    Tree *n, *t, *l, *r;
    char *oops;
    int oopscol;
    int left, right;
    OPR *op;

    /* Skip whitespace */
    skipws();

    /* Parse a single term: a constants, variable, or parenthasis, possibly
                with a preceded by prefix operators. */
    switch(*ptr)
    {
        case '%': 							/* Binary */
        {
            int num=0;
            ++ptr, ++col;
            while(*ptr=='0' || *ptr=='1') num=num*2+*ptr++-'0', ++col;
            n=mk(Intid,num);
        }
        break;
        
        case '@': 							/* Octal */
        {
            int num=0;
            ++ptr, ++col;
            while(*ptr>='0' && *ptr<='7') num=num*8+*ptr++-'0', ++col;
            n=mk(Intid,num);
        }
        break;

        case '$': 							/* Hex */
        {
            int num=0;
            ++ptr, ++col;
            while(*ptr>='0' && *ptr<='9' ||
                                    *ptr>='a' && *ptr<='f' ||
                                    *ptr>='A' && *ptr<='F')
                if(*ptr>='0' && *ptr<='9') num=num*16+*ptr++-'0', ++col;
                else if(*ptr>='a' && *ptr<='f') num=num*16+*ptr++-'a'+10, ++col;
                else if(*ptr>='A' && *ptr<='F') num=num*16+*ptr++-'A'+10, ++col;
            n=mk(Intid,num);
        }
        break;

        case '0': case '1': case '2': case '3': case '4':		/* Decimal */
        case '5': case '6': case '7': case '8': case '9':
        {
            int num=0, x=col;
            do num=num*10+*ptr++-'0', ++col; while(isdigit(*ptr));
/*
            if(*ptr=='.' || *ptr=='e' || *ptr=='E')
            {
                ptr-=col-x;
                col=x;
                goto flt;
            }
*/
            n=mk(Intid,num);
        }
        break;

        case '.':							/* Floating */
        {
/*
            if(ptr[1]<'0' || ptr[1]>'9')
            { */ /* Current location */
                ++ptr; ++col;
                if(cursect)
                    n=mk(Labelid,cursect->last->len,cursect->last,cursect);
                else
                    error0("No section for '.'"),
                    n=mk(Intid,0);
/*    }
            else
            {
                int x;
                double dbl;
                flt:
                x=0;
                while(ptr[x]>='0' && ptr[x]<='9') ++x;
                if(ptr[x]=='.')
                {
                    ++x;
                    while(ptr[x]>='0' && ptr[x]<='9') ++x;
                }
                if(ptr[x]=='e' || ptr[x]=='E')
                {
                    ++x;
                    if(ptr[x]=='-' || ptr[x]=='+') ++x;
                    while(ptr[x]>='0' && ptr[x]<='9') ++x;
                }
                sscanf(ptr,"%lf",&dbl);
                ptr+=x; col+=x;
                n=mk(Floatid,dbl);
            } */
        }
        break;

        case '\'':							/* Character */
        {
            int num=0;
            ++ptr, ++col;
            num=escape();
            if(*ptr=='\'') ++ptr, ++col;
            n=mk(Intid,num);
        }
        break;

        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':	/* Word */
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case '_':
            if(symtab)
            {
                n=mk(Symbolid,findsym(symtab,word()));
                addref(n->Symbol.sym);
            }
            else
                n=mk(Nameid,word());
        break;

        case '"':							/* String */
        {
            int x;
            for(x=0,++ptr,++col;*ptr!='"' && *ptr && *ptr!='\r' && *ptr!='\n';++x)
            {
                if(x==bufsiz)
                    if(strbuf) strbuf=(char *)realloc(strbuf,bufsiz<<=1);
                    else strbuf=(char *)malloc(bufsiz=256);
                strbuf[x]=escape();
            }
            if(*ptr!='"') error0("unterminated string constant");
            else ++ptr, ++col;
            n=mk(Stringid,strbuf,x);
        }
        break;

        default:							/* Operator? */
        parseopr:
        oops=ptr; oopscol=col; skipws(); op=opr();
        if(op && (op=op->prefix))
            switch(op->meth&15)
            {
                case 5: /* Parenthasis */
/*    n=mk1(op->func,parseexpr(symtab,0)); */
                n=parseexpr(symtab,0);
                if(*ptr!=')') error0("missing )");
                else ++ptr, ++col;
                break;

                case 0: /* Normal prefix operator */
                n=mk(op->func,parseexpr(symtab,op->prec));
                break;
            }
        else n=0, ptr=oops, col=oopscol;
    }
    if(!n) return 0;						/* No expr */

    /* Infix/Postfix operators */
    loop:

    oops=ptr; oopscol=col;
    left=col;
    skipws();
    left=col-left;

    op=opr();

    right=col;
    skipws();
    if(*ptr) right=col-right;
    else right=32767;

    if(op && op->infix && ( /* !op->prefix || */ left<=right) &&
                (op->infix->prec>prec || op->infix->prec==prec && op->infix->ass))
    {
        char *s;
        op=op->infix;
        switch(op->meth)
        {
            case 5: /* Function call */
            n=mk(op->func,n,parseexpr(symtab,0));
            if(*ptr==')') ++ptr, ++col;
            else error0("missing )");
            break;

            case 2: /* ?: operator */
            l=parseexpr(symtab,op->prec);
            skipws();
            if(*ptr==':') ++ptr, ++col, r=parseexpr(symtab,op->prec);
            else error0("missing :"), r=0;
            n=mk(op->func,n,l,r);
            break;

            case 0: /* Normal infix operator */
            n=mk(op->func,n,parseexpr(symtab,op->prec));
            break;
        }
        goto loop;
    }
    else if(op && op->postfix && ( /* !op->prefix || */ left<=right) &&
                                    (op->postfix->prec>prec ||
                                        op->postfix->prec==prec && op->postfix->ass))
    {
        op=op->postfix;
        n=mk(op->func,n);
        goto loop;
    }
    else ptr=oops, col=oopscol;
    return n;
}

/* Parse an expression.  Skip trailing whitespace. */

Tree *expr(struct symtab *symtab, char **s)
{
    Tree *r;
    ptr= *s;
    col=0;
    lvl= -1;
    r=parseexpr(symtab,0);
    skipws();
    *s= ptr;
    return r;
}
