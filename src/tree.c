/* Object oriented parse tree */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "input.h"
#include "output.h"

#include "symtab.h"
#include "frag.h"
#include "ulink.h"

#include "tree.h"

Tree *mk(struct Nodeid *id, ...)
{
    Tree *n = (Tree *)malloc(id->size);
    Tree *t;
    va_list ap;
    va_start (ap, id);
    t = id->mk(n, id, ap);
    va_end(ap);
    return t;
}

void indent(FILE *file,int lvl)
{
    while(lvl--) fprintf(file," ");
}

void showNode(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file," = ");
    else indent(file,lvl);
    fprintf(file,"%s\n",n->Node.id->name);
}

Tree *mkNode(Tree *n,struct Nodeid *id,va_list ap)
{
    n->Node.id=id;
    return n;
}

int rmNode(Tree *n)
{
    free(n);
    return 0;
}

Tree *dupNode(Tree *n)
{
    return mk(n->Node.id);
}

void emitNode(Tree *n)
{
    emitc(n->Node.id->id2);
}

Tree *loadNode(struct Nodeid *id)
{
    return mk(id);
}

void showInt(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file," = ");
    else indent(file,lvl);
    fprintf(file,"%d\n",n->Int.val);
}

Tree *mkInt(Tree *n,struct Nodeid *id,va_list ap)
{
    int val = (int)va_arg(ap, int);
    mkNode(n,id,ap);
    n->Int.val=val;
    return n;
}

Tree *dupInt(Tree *n)
{
    return mk(n->Node.id,n->Int.val);
}

void emitInt(Tree *n)
{
    emitNode(n);
    emitnum(n->Int.val);
}

Tree *simpInt(Tree *n)
{
    return n;
}

Tree *loadInt(struct Nodeid *id)
{
    int val=GETNUM();
    return mk(id,val);
}

void showString(Tree *n,FILE *file,int flg,int lvl)
{
    int x;
    if(flg) fprintf(file," = ");
    else indent(file,lvl);
    fprintf(file,"\"");
    for(x=0;x!=n->String.len;++x)
        if(n->String.val[x]>=' ' && n->String.val[x]<='~') fprintf(file,"%c",n->String.val[x]);
        else fprintf(file,"\\x%2.2x",((unsigned char *)n->String.val)[x]);
    fprintf(file,"\"\n");
}

Tree *mkString(Tree *n,struct Nodeid *id,va_list ap)
{
    char *str = (char *)va_arg(ap, char *);
    int len = (int)va_arg(ap, int);
    mkNode(n,id,ap);
    n->String.val=malloc(len+1);
    n->String.len=len;
    memcpy(n->String.val,str,len);
    n->String.val[n->String.len]=0;
    return n;
}

Tree *dupString(Tree *n)
{
    return mk(n->Node.id,n->String.val,n->String.len);
}

void emitString(Tree *n)
{
    emitNode(n);
    emitsn(n->String.val,n->String.len);
}

Tree *simpString(Tree *n)
{
    return n;
}

Tree *loadString(struct Nodeid *id)
{
    int len=GETNUM();
    Tree *t=mk(id,recbuf+recidx,len-1);
    recidx+=len;
    return t;
}

int rmString(Tree *n)
{
    free(n->String.val);
    return rmNode(n);
}

void showName(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file," = ");
    else indent(file,lvl);
    fprintf(file,"name [%s]\n",n->Name.name);
}

Tree *mkName(Tree *n,struct Nodeid *id,va_list ap)
{
    char *str = (char *)va_arg(ap, char *);
    mkNode(n,id,ap);
    n->Name.name=malloc(strlen(str)+1);
    strcpy(n->Name.name,str);
    return n;
}

Tree *dupName(Tree *n)
{
    return mk(n->Node.id,n->Name.name);
}

int rmName(Tree *n)
{
    free(n->Name.name);
    return rmNode(n);
}

void emitName(Tree *n)
{
    emitNode(n);
    emits(n->Name.name);
}

Tree *simpName(Tree *n)
{
    return n;
}

Tree *loadName(struct Nodeid *id)
{
    return mk(id,getword());
}

void showSymbol(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file," = ");
    else indent(file,lvl);
    fprintf(file,"sym [%p]\n",n->Symbol.sym);
}

Tree *mkSymbol(Tree *n,struct Nodeid *id,va_list ap)
{
    void *sym = (void *)va_arg(ap, void *);
    mkNode(n,id,ap);
    n->Symbol.sym=sym;
    return n;
}

Tree *dupSymbol(Tree *n)
{
    return mk(n->Node.id,n->Symbol.sym);
}

int rmSymbol(Tree *n)
{
    return rmNode(n);
}

void emitSymbol(Tree *n)
{
    emitNode(n);
    emitsymbol(n->Symbol.sym);
}

Tree *simpSymbol(Tree *n)
{
    if(((struct symbol *)n->Symbol.sym)->v)
    {
        Tree *r=simp(dup(((struct symbol *)n->Symbol.sym)->v));
        rm(n);
        return r;
    }
    else
        return n;
}

Tree *loadSymbol(struct Nodeid *id)
{
    int x=GETNUM();
    return mk(id,sym[x]);
}

void showLabel(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file," = ");
    else indent(file,lvl);
    fprintf(file,"label %lu[%s]\n",n->Label.offset,n->Label.frag->owner->name);
}

Tree *mkLabel(Tree *n,struct Nodeid *id,va_list ap)
{
    unsigned long offset = (unsigned long)va_arg(ap, unsigned long);
    struct frag *frag = (struct frag *)va_arg(ap, struct frag *);
    struct section *sec = (struct section *)va_arg(ap, struct section *);
    mkNode(n,id,ap);
    n->Label.offset=offset;
    n->Label.frag=frag;
    n->Label.sec=sec;
    return n;
}

Tree *dupLabel(Tree *n)
{
    return mk(n->Node.id,n->Label.offset,n->Label.frag,n->Label.sec);
}

int rmLabel(Tree *n)
{
    return rmNode(n);
}

void emitLabel(Tree *n)
{
    emitNode(n);
    emitnum(n->Label.frag->owner->no);		/* Section no. */
    emitnum(n->Label.frag->offset+n->Label.offset);	/* Offset from start of section */
}

Tree *simpLabel(Tree *n)
{
    if(n->Label.sec->set)
    {
        Tree *r=mk(Intid,(int)(n->Label.sec->addr+n->Label.offset));
        rm(n);
        return r;
    }
    else
        return n;
}

Tree *loadLabel(struct Nodeid *id)
{
    int x=GETNUM();
    int ofst=GETNUM();
    ofst+=sec[x]->offset;
    return mk(id,ofst,NULL,sec[x]);
}

void showSingle(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file,"\n");
    indent(file,lvl); fprintf(file,"%s\n",n->Single.id->name);
    indent(file,lvl+1); fprintf(file,".operand"); show(n->Single.operand,file,1,lvl+2);
}

Tree *mkSingle(Tree *n,struct Nodeid *id,va_list ap)
{
    Tree *t = (Tree *)va_arg(ap, Tree *);
    mkNode(n,id,ap);
    n->Single.operand=t;
    return n;
}

Tree *dupSingle(Tree *n)
{
    return mk(n->Node.id,dup(n->Single.operand));
}

int rmSingle(Tree *n)
{
    rm(n->Single.operand);
    return rmNode(n);
}

void emitSingle(Tree *n)
{
    emitNode(n);
    emit(n->Single.operand);
}

Tree *loadSingle(struct Nodeid *id)
{
    return mk(id,interm());
}

void showDouble(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file,"\n");
    indent(file,lvl); fprintf(file,"%s\n",n->Node.id->name);
    indent(file,lvl+1); fprintf(file,".left"); show(n->Double.left,file,1,lvl+2);
    indent(file,lvl+1); fprintf(file,".right"); show(n->Double.right,file,1,lvl+2);
}

Tree *mkDouble(Tree *n,struct Nodeid *id,va_list ap)
{
    Tree *left = (Tree *)va_arg(ap, Tree *);
    Tree *right = (Tree *)va_arg(ap, Tree *);
    mkNode(n,id,ap);
    n->Double.left=left;
    n->Double.right=right;
    return n;
}

Tree *dupDouble(Tree *n)
{
    return mk(n->Node.id,dup(n->Double.left),dup(n->Double.right));
}

int rmDouble(Tree *n)
{
    rm(n->Double.left);
    rm(n->Double.right);
    return rmNode(n);
}

void emitDouble(Tree *n)
{
    emitNode(n);
    emit(n->Double.left);
    emit(n->Double.right);
}

Tree *loadDouble(struct Nodeid *id)
{
    Tree *l=interm();
    Tree *r=interm();
    return mk(id,l,r);
}

void showTriple(Tree *n,FILE *file,int flg,int lvl)
{
    if(flg) fprintf(file,"\n");
    indent(file,lvl); fprintf(file,"%s\n",n->Node.id->name);
    indent(file,lvl+1); fprintf(file,".test"); show(n->Triple.test,file,1,lvl+2);
    indent(file,lvl+1); fprintf(file,".left"); show(n->Triple.left,file,1,lvl+2);
    indent(file,lvl+1); fprintf(file,".right"); show(n->Triple.right,file,1,lvl+2);
}

Tree *mkTriple(Tree *n,struct Nodeid *id,va_list ap)
{
    Tree *test = (Tree *)va_arg(ap, Tree *);
    Tree *left = (Tree *)va_arg(ap, Tree *);
    Tree *right = (Tree *)va_arg(ap, Tree *);
    mkNode(n,id,ap);
    n->Triple.test=test;
    n->Triple.left=left;
    n->Triple.right=right;
    return n;
}

Tree *dupTriple(Tree *n)
{
    return mk(n->Node.id,dup(n->Triple.test),dup(n->Triple.left),dup(n->Triple.right));
}

int rmTriple(Tree *n)
{
    rm(n->Triple.test);
    rm(n->Triple.left);
    rm(n->Triple.right);
    return rmNode(n);
}

void emitTriple(Tree *n)
{
    emitNode(n);
    emit(n->Triple.test);
    emit(n->Triple.left);
    emit(n->Triple.right);
}

Tree *loadTriple(struct Nodeid *id)
{
    Tree *t=interm();
    Tree *l=interm();
    Tree *r=interm();
    return mk(id,t,l,r);
}

struct Nodeid Intid[1]=
{
    /* id2: */ iINT,			/* size: */ sizeof(Int),
    /* mk: */ mkInt,			/* dup: */ dupInt,
    /* name: */ "Int",			/* show: */ showInt,
    /* rm: */ rmNode,			/* emit: */ emitInt,
    /* load: */ loadInt,			/* simp: */ simpInt,
};

struct Nodeid Stringid[1]=
{
    /* id2: */ iSTRING,			/* size: */ sizeof(String),
    /* mk: */ mkString,			/* dup: */ dupString,
    /* name: */ "String",			/* show: */ showString,
    /* rm: */ rmString,			/* emit: */ emitString,
    /* load: */ loadString,		/* simp: */ simpString,
};

struct Nodeid Nameid[1]=
{
    /* id2: */ iNAME,			/* size: */ sizeof(Name),
    /* mk: */ mkName,			/* dup: */ dupName,
    /* name: */ "name",			/* show: */ showName,
    /* rm: */ rmName,			/* emit: */ emitName,
    /* load: */ loadName,			/* simp: */ simpName,
};

struct Nodeid Symbolid[1]=
{
    /* id2: */ iSYMBOL,			/* size: */ sizeof(Symbol),
    /* mk: */ mkSymbol,			/* dup: */ dupSymbol,
    /* name: */ "Symbol",			/* show: */ showSymbol,
    /* rm: */ rmSymbol,			/* emit: */ emitSymbol,
    /* load: */ loadSymbol,		/* simp: */ simpSymbol,
};

struct Nodeid Labelid[1]=
{
    /* id2: */ iLABEL,			/* size: */ sizeof(Label),
    /* mk: */ mkLabel,			/* dup: */ dupLabel,
    /* name: */ "Label",			/* show: */ showLabel,
    /* rm: */ rmLabel,			/* emit: */ emitLabel,
    /* load: */ loadLabel,			/* simp: */ simpLabel,
};

Tree *simpNot(Tree *n)
{
    n->Not.operand=simp(n->Not.operand);
    if(n->Not.operand->Int.id==Intid)
    {
        Tree *r=n->Not.operand;
        n->Not.operand=0;
        rm(n);
        r->Int.val= !r->Int.val;
        return r;
    }
    else
        return n;
}

struct Nodeid Notid[1]=
{
    /* id2: */ iNOT,			/* size: */ sizeof(Not),
    /* mk: */ mkSingle,			/* dup: */ dupSingle,
    /* name: */ "Not",			/* show: */ showSingle,
    /* rm: */ rmSingle,			/* emit: */ emitSingle,
    /* load: */ loadSingle,		/* simp: */ simpNot,
};

Tree *simpCom(Tree *n)
{
    n->Com.operand=simp(n->Com.operand);
    if(n->Com.operand->Int.id==Intid)
    {
        Tree *r=n->Com.operand;
        n->Com.operand=0;
        rm(n);
        r->Int.val= ~r->Int.val;
        return r;
    }
    else
        return n;
}

struct Nodeid Comid[1]=
{
    /* id2: */ iCOM,			/* size: */ sizeof(Com),
    /* mk: */ mkSingle,			/* dup: */ dupSingle,
    /* name: */ "Com",			/* show: */ showSingle,
    /* rm: */ rmSingle,			/* emit: */ emitSingle,
    /* load: */ loadSingle,		/* simp: */ simpCom,
};

Tree *simpNeg(Tree *n)
{
    n->Neg.operand=simp(n->Neg.operand);
    if(n->Neg.operand->Int.id==Intid)
    {
        Tree *r=n->Neg.operand;
        n->Neg.operand=0;
        rm(n);
        r->Int.val= -r->Int.val;
        return r;
    }
    else
        return n;
}

struct Nodeid Negid[1]=
{
    /* id2: */ iNEG,			/* size: */ sizeof(Neg),
    /* mk: */ mkSingle,			/* dup: */ dupSingle,
    /* name: */ "Neg",			/* show: */ showSingle,
    /* rm: */ rmSingle,			/* emit: */ emitSingle,
    /* load: */ loadSingle,		/* simp: */ simpNeg,
};

Tree *simpMul(Tree *n)
{
    n->Mul.left=simp(n->Mul.left);
    n->Mul.right=simp(n->Mul.right);
    if(n->Mul.left->Int.id==Intid && n->Mul.right->Int.id==Intid)
    {
        Tree *r=n->Mul.left;
        n->Mul.left=0;
        r->Int.val *= n->Mul.right->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Mulid[1]=
{
    /* id2: */ iMUL,			/* size: */ sizeof(Mul),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Mul",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpMul,
};

Tree *simpDiv(Tree *n)
{
    n->Div.left=simp(n->Div.left);
    n->Div.right=simp(n->Div.right);
    if(n->Div.left->Int.id==Intid && n->Div.right->Int.id==Intid)
    {
        Tree *r=n->Div.left;
        n->Div.left=0;
        r->Int.val /= n->Div.right->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Divid[1]=
{
    /* id2: */ iDIV,			/* size: */ sizeof(Div),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Div",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpDiv,
};

Tree *simpMod(Tree *n)
{
    n->Mod.left=simp(n->Mod.left);
    n->Mod.right=simp(n->Mod.right);
    if(n->Mod.left->Int.id==Intid && n->Mod.right->Int.id==Intid)
    {
        Tree *r=n->Mod.left;
        n->Mod.left=0;
        r->Int.val %= n->Mod.right->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Modid[1]=
{
    /* id2: */ iMOD,			/* size: */ sizeof(Mod),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Mod",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpMod,
};

Tree *simpAdd(Tree *n)
{
    n->Add.left=simp(n->Add.left);
    n->Add.right=simp(n->Add.right);
    if(n->Add.left->Int.id==Intid && n->Add.right->Int.id==Intid)
    {
        Tree *r=n->Add.left;
        n->Add.left=0;
        r->Int.val += n->Add.right->Int.val;
        rm(n);
        return r;
    }
    else if(n->Add.left->Label.id==Labelid && n->Add.right->Int.id==Intid)
    {
        Tree *r=n->Add.left;
        n->Add.left=0;
        r->Label.offset += n->Add.right->Int.val;
        rm(n);
        return r;
    }
    else if(n->Add.left->Int.id==Intid && n->Add.right->Label.id==Labelid)
    {
        Tree *r=n->Add.right;
        n->Add.right=0;
        r->Label.offset += n->Add.left->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Addid[1]=
{
    /* id2: */ iADD,			/* size: */ sizeof(Add),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Add",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpAdd,
};

Tree *simpSub(Tree *n)
{
    n->Sub.left=simp(n->Sub.left);
    n->Sub.right=simp(n->Sub.right);
    if(n->Sub.left->Int.id==Intid && n->Sub.right->Int.id==Intid)
    {
        Tree *r=n->Sub.left;
        n->Sub.left=0;
        r->Int.val -= n->Sub.right->Int.val;
        rm(n);
        return r;
    }
    else if(n->Sub.left->Label.id==Labelid && n->Sub.right->Int.id==Intid)
    {
        Tree *r=n->Sub.left;
        n->Sub.left=0;
        r->Label.offset -= n->Sub.right->Int.val;
        rm(n);
        return r;
    }
    else if(n->Sub.left->Label.id==Labelid && n->Sub.right->Label.id==Labelid &&
                                    ( n->Sub.left->Label.frag ? n->Sub.left->Label.frag==n->Sub.right->Label.frag
                                                                                                                            : n->Sub.left->Label.sec==n->Sub.right->Label.sec ) )
    {
        Tree *r = mk(Intid,(int)(n->Sub.left->Label.offset-n->Sub.right->Label.offset));
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Subid[1]=
{
    /* id2: */ iSUB,			/* size: */ sizeof(Sub),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Sub",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpSub,
};

Tree *simpAnd(Tree *n)
{
    n->And.left=simp(n->And.left);
    n->And.right=simp(n->And.right);
    if(n->And.left->Int.id==Intid && n->And.right->Int.id==Intid)
    {
        Tree *r=n->And.left;
        n->And.left=0;
        r->Int.val &= n->And.right->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Andid[1]=
{
    /* id2: */ iAND,			/* size: */ sizeof(And),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "And",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpAnd,
};

Tree *simpOr(Tree *n)
{
    n->Or.left=simp(n->Or.left);
    n->Or.right=simp(n->Or.right);
    if(n->Or.left->Int.id==Intid && n->Or.right->Int.id==Intid)
    {
        Tree *r=n->Or.left;
        n->Or.left=0;
        r->Int.val |= n->Or.right->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Orid[1]=
{
    /* id2;: */ iOR,			/* size: */ sizeof(Or),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Or",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpOr,
};

Tree *simpXor(Tree *n)
{
    n->Xor.left=simp(n->Xor.left);
    n->Xor.right=simp(n->Xor.right);
    if(n->Xor.left->Int.id==Intid && n->Xor.right->Int.id==Intid)
    {
        Tree *r=n->Xor.left;
        n->Xor.left=0;
        r->Int.val ^= n->Xor.right->Int.val;
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Xorid[1]=
{
    /* id2: */ iXOR,			/* size: */ sizeof(Xor),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Xor",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpXor,
};

Tree *simpEq(Tree *n)
{
    n->Eq.left=simp(n->Eq.left);
    n->Eq.right=simp(n->Eq.right);
    if(n->Eq.left->Int.id==Intid && n->Eq.right->Int.id==Intid)
    {
        Tree *r=n->Eq.left;
        n->Eq.left=0;
        r->Int.val=(r->Int.val==n->Eq.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Eqid[1]=
{
    /* id2: */ iEQ,			/* size: */ sizeof(Eq),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Eq",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpEq,
};

Tree *simpNe(Tree *n)
{
    n->Ne.left=simp(n->Ne.left);
    n->Ne.right=simp(n->Ne.right);
    if(n->Ne.left->Int.id==Intid && n->Ne.right->Int.id==Intid)
    {
        Tree *r=n->Ne.left;
        n->Ne.left=0;
        r->Int.val=(r->Int.val!=n->Ne.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Neid[1]=
{
    /* id2: */ iNE,			/* size: */ sizeof(Ne),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Ne",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpNe,
};

Tree *simpGt(Tree *n)
{
    n->Gt.left=simp(n->Gt.left);
    n->Gt.right=simp(n->Gt.right);
    if(n->Gt.left->Int.id==Intid && n->Gt.right->Int.id==Intid)
    {
        Tree *r=n->Ge.left;
        n->Ge.left=0;
        r->Int.val=(r->Int.val>n->Ge.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Gtid[1]=
{
    /* id2: */ iGT,			/* size: */ sizeof(Gt),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Gt",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpGt,
};

Tree *simpGe(Tree *n)
{
    n->Ge.left=simp(n->Ge.left);
    n->Ge.right=simp(n->Ge.right);
    if(n->Ge.left->Int.id==Intid && n->Ge.right->Int.id==Intid)
    {
        Tree *r=n->Ge.left;
        n->Ge.left=0;
        r->Int.val=(r->Int.val >= n->Ge.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Geid[1]=
{
    /* id2: */ iGE,			/* size: */ sizeof(Ge),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Ge",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpGe,
};

Tree *simpLt(Tree *n)
{
    n->Lt.left=simp(n->Lt.left);
    n->Lt.right=simp(n->Lt.right);
    if(n->Lt.left->Int.id==Intid && n->Lt.right->Int.id==Intid)
    {
        Tree *r=n->Lt.left;
        n->Lt.left=0;
        r->Int.val=(r->Int.val<n->Lt.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Ltid[1]=
{
    /* id2: */ iLT,			/* size: */ sizeof(Lt),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Lt",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpLt,
};

Tree *simpLe(Tree *n)
{
    n->Le.left=simp(n->Le.left);
    n->Le.right=simp(n->Le.right);
    if(n->Le.left->Int.id==Intid && n->Le.right->Int.id==Intid)
    {
        Tree *r=n->Le.left;
        n->Le.left=0;
        r->Int.val=(r->Int.val <= n->Le.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Leid[1]=
{
    /* id2: */ iLE,			/* size: */ sizeof(Le),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Le",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpLe,
};

Tree *simpLand(Tree *n)
{
    n->Land.left=simp(n->Land.left);
    n->Land.right=simp(n->Land.right);
    if(n->Land.left->Int.id==Intid && n->Land.right->Int.id==Intid)
    {
        Tree *r=n->Land.left;
        n->Land.left=0;
        r->Int.val=(r->Int.val && n->Land.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Landid[1]=
{
    /* id2: */ iLAND,			/* size: */ sizeof(Land),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Land",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpLand,
};

Tree *simpLor(Tree *n)
{
    n->Lor.left=simp(n->Lor.left);
    n->Lor.right=simp(n->Lor.right);
    if(n->Lor.left->Int.id==Intid && n->Lor.right->Int.id==Intid)
    {
        Tree *r=n->Lor.left;
        n->Lor.left=0;
        r->Int.val=(r->Int.val || n->Lor.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Lorid[1]=
{
    /* id2: */ iLOR,			/* size: */ sizeof(Lor),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Lor",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpLor,
};

Tree *simpShr(Tree *n)
{
    n->Shr.left=simp(n->Shr.left);
    n->Shr.right=simp(n->Shr.right);
    if(n->Shr.left->Int.id==Intid && n->Shr.right->Int.id==Intid)
    {
        Tree *r=n->Shr.left;
        n->Shr.left=0;
        r->Int.val=(r->Int.val >> n->Shr.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Shrid[1]=
{
    /* id2: */ iSHR,			/* size: */ sizeof(Shr),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Shr",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpShr,
};

Tree *simpShl(Tree *n)
{
    n->Shl.left=simp(n->Shl.left);
    n->Shl.right=simp(n->Shl.right);
    if(n->Shl.left->Int.id==Intid && n->Shl.right->Int.id==Intid)
    {
        Tree *r=n->Shl.left;
        n->Shl.left=0;
        r->Int.val=(r->Int.val << n->Shl.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Shlid[1]=
{
    /* id2: */ iSHL,			/* size: */ sizeof(Shl),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Shl",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ simpShl,
};

struct Nodeid Parenid[1]=
{
    /* id2: */ iPAREN,			/* size: */ sizeof(Paren),
    /* mk: */ mkDouble,			/* dup: */ dupDouble,
    /* name: */ "Paren",			/* show: */ showDouble,
    /* rm: */ rmDouble,			/* emit: */ emitDouble,
    /* load: */ loadDouble,		/* simp: */ NULL,
};

Tree *simpMux(Tree *n)
{
    n->Mux.test=simp(n->Mux.test);
    n->Mux.left=simp(n->Mux.left);
    n->Mux.right=simp(n->Mux.right);
    if(n->Mux.test->Int.id==Intid && n->Mux.left->Int.id==Intid && n->Mux.right->Int.id==Intid)
    {
        Tree *r=n->Mux.test;
        n->Mux.test=0;
        r->Int.val=(r->Int.val ? n->Mux.left->Int.val : n->Mux.right->Int.val);
        rm(n);
        return r;
    }
    else
        return n;
}

struct Nodeid Muxid[1]=
{
    /* id2: */ iMUX,			/* size: */ sizeof(Mux),
    /* mk: */ mkTriple,			/* dup: */ dupTriple,
    /* name: */ "Mux",			/* show: */ showTriple,
    /* rm: */ rmTriple,			/* emit: */ emitTriple,
    /* load: */ loadTriple,		/* simp: */ simpMux,
};

struct Nodeid *idtable[]=
{
    0,
    Intid,
    Stringid,
    Nameid,
    Symbolid,
    Labelid,
    0,
    Notid,
    Comid,
    Negid,
    0,
    Mulid,
    Divid,
    Modid,
    Addid,
    Subid,
    Andid,
    Orid,
    Xorid,
    Eqid,
    Neid,
    Gtid,
    Ltid,
    Geid,
    Leid,
    Landid,
    Lorid,
    Shrid,
    Shlid,
    Parenid,
    0,
    Muxid,
};

Tree *interm(void)
{
    int c=GETC();
    struct Nodeid *id;
    id=idtable[c-0x80];
    return id->load(id);
}
