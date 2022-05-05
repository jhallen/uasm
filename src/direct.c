/* Simple directives */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "expr.h"
#include "error.h"
#include "macro.h"
#include "frag.h"
#include "symtab.h"
#include "parse.h"
#include "uasm.h"

void doemit(struct macro *macro, char *s)
{
    int x;
    if(!cursect)
    {
        error0("Need a section before .emit");
        return;
    }
    if(!cursect->last->data)
        newfrag(cursect,1,1);
    do
    {
        Tree *v=expr(symtab,&s);
        struct fixup *f;
        v=simp(v);
        if(!v)
        {
            error0("Expression missing after .emit");
            return;
        }
        else if(v->Int.id==Intid)
        {
            catbyte(cursect,v->Int.val);	/* Yes. */
            rm(v);
        }
/* A string?
        else if(v->String.id==Stringid)
        {
            int y;
            for(y=0;y!=v->String.len;++y) catbyte(cursect,v->String.val[y]);
            rm(v);
        }
*/
        else
        {
            char msgbuf[512];
            srcref(msgbuf);
            fixup(cursect,fBYTE,cursect->last->len,v,mk(Stringid,msgbuf,strlen(msgbuf)));
            catbyte(cursect,0);				/* Fixup */
        }
    }
        while(*s==',' && *(s=parseskip(++s)));
    if(*s && *s!=';')
    {
        error0("Junk after expression");
        return;
    }
}

void dopublic(struct macro *macro, char *s)
{
    do
    {
        Tree *v=expr(symtab,&s);
        if(!v)
        {
            error0("Symbol missing after .public");
            return;
        }
        else if(v->Symbol.id==Symbolid)
        {
            struct symbol *sym=v->Symbol.sym;
            sym->pub=1;
            rm(v);
        }
        else
        {
            error0("Arguments to .public must be symbols");
            rm(v);
        }
    }
        while(*s==',' && *(s=parseskip(++s)));
    if(*s && *s!=';')
    {
        error0("Junk after symbol");
        return;
    }
}

void dosect(struct macro *macro, char *s)
{
    Tree *v;
    v=expr(symtab,&s);
    v=simp(v);
    if(!v) error0("Missing section name");
    else if(v->String.id!=Stringid)
        error0("Section name must be a string"), rm(v);
    else
    {
        cursect=findsect(v->String.val);
        rm(v);
        if(*s && *s!=';') error0("Extra junk after section name");
    }
}

void doequ(struct macro *macro, char *s)
{
    Tree *sy;
    Tree *v;
    sy=expr(symtab,&s);
    if(!sy)
    {
        error0("Need symbol to equate");
        return;
    }
    if(*s==',') ++s;
    v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0("Need value to equate to symbol");
        return;
    }
    if(*s && *s!=';')
        error0("Equate only needs two args");
    if(sy->Symbol.id==Symbolid)
    {
        struct symbol *sym=sy->Symbol.sym;
        if(sym->v) error0("Symbol can be equated only once"), rm(v);
        else
        {
            char tmp[512];
            sym->v=v, sym->state=2;
            srcref(tmp);
            sym->defline=strdup(tmp);
        }
    }
    else error0("Left side of equ must be a symbol"), rm(v);
}

void doset(struct macro *macro, char *s)
{
    Tree *sy;
    Tree *v;
    sy=expr(symtab,&s);
    if(!sy)
    {
        error0("Need symbol to set");
        return;
    }
    if(*s==',') ++s;
    v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0("Need value to set to symbol");
        return;
    }
    if(*s && *s!=';')
        error0("Set only needs two args");
    if(sy->Symbol.id==Symbolid)
    {
        struct symbol *sym=sy->Symbol.sym;
        if(sym->state==2) error0("Can not set equated symbols"), rm(v);
        else
        {
            char tmp[512];
            rm(sym->v), sym->v=v, sym->state=1;
            srcref(tmp);
            sym->defline=strdup(tmp);
        }
    }
    else error0("Left side of .set must be a symbol"), rm(v);
}

void dospace(struct macro *mac, char *s)
{
    Tree *v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0(".space requires an argument");
        return;
    }
    if(!cursect)
    {
        error0(".space needs to be in a section");
        rm(v);
        return;
    }
    if(*s && *s!=';')
        error0(".space needs only one argument");
    if(v->Int.id!=Intid)
        error0("argument for .space must evaluate to an integer");
    else
    {
        if(cursect->last->data) newfrag(cursect,1,0);
        cursect->last->len+=v->Int.val;
        cursect->offset+=v->Int.val;
    }
    rm(v);
}

void doalign(struct macro *mac, char *s)
{
    Tree *v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0(".align requires an argument");
        return;
    }
    if(!cursect)
    {
        error0(".align needs to be in a section");
        rm(v);
        return;
    }
    if(*s && *s!=';')
        error0(".align needs only one argument");
    if(v->Int.id!=Intid)
        error0("argument for .align must evaluate to an integer");
    else
    {
        unsigned n=v->Int.val;
        if(cursect->last->align<n || cursect->last->align%n)
            newfrag(cursect,n,1);
    }
    rm(v);
}

void doerrif(struct macro *mac, char *s)
{
    Tree *v;
    Tree *msg;
    v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0("Expression missing after .errif");
        return;
    }
    if(*s==',')
    {
        ++s;
        msg=expr(symtab,&s);
        msg=simp(msg);
        if(!msg)
        {
            error0("Message missing after .errif");
            return;
        }
        else if(msg->String.id!=Stringid)
        {
            error0("Message needs to be a string .errif");
            return;
        }
        else
        { /* We have expression and message */
            if(v->Int.id==Intid)
            {
                if(v->Int.val) error0(msg->String.val);
            }
            else
            {
                char tmp[512];
                char msgbuf[512];
                srcref(tmp);
                sprintf(msgbuf,"%s%s",tmp,msg->String.val);
                free(msg->String.val);
                msg->String.val=strdup(msgbuf);
                msg->String.len=strlen(msgbuf);
                fixup(cursect,fCHECK,cursect->last->len,v,msg);
            }
        }
    }
    else
        error0("Message missing after .errif");
}

/* Variable length code generation */

void dosmall(struct macro *mac, char *s)
{
    Tree *v;
    v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0("Expression missing after .small");
        return;
    }
    else
    {
    }
}

void dobigger(struct macro *mac, char *s)
{
    Tree *v;
    v=expr(symtab,&s);
    v=simp(v);
    if(!v)
    {
        error0("Expression missing after .bigger");
        return;
    }
    else
    {
    }
}

void dobiggest(struct macro *mac, char *s)
{
    s=parseskip(s);
    if(*s && *s!=';')
    {
        error0("Junk after .biggest");
        return;
    }
}

void dodone(struct macro *mac, char *s)
{
    s=parseskip(s);
    if(*s && *s!=';')
    {
        error0("Junk after .done");
        return;
    }
}
