/* Object oriented parse-tree */

#ifndef _Itree
#define _Itree 1

#include <stdarg.h>

/* 'Tree' is a union of all possible tree node types.  If you add a new tree
    * node type you must update this union, the secondary node identifier enum
    * and secondary to primary identifier translation table.  See the end of this
    * file for more information.
    */

typedef union tree Tree;

/* Base class for parse tree nodes */

    /* Note that the class name 'Node' consistantly appears in each name
        * associated with the class, including structure names, typedef names,
        * #define names, etc.  This allows us to use a text editor to easily change
        * the name of the class.
        */

    typedef struct Nodevars Node;	/* Generic parse tree node object type */

    /* Per-class members.  These remain constant for each object instance of the
        * the class.  The per-class members can be accessed by dereferencing the 'id'
        * member of object.
        *
        * Object per-class members are defined in a preprocessor definition so that
        * derived classes can inherit these members simply by naming the
        * preprocessor '#define' name in their per-class structure.
        */

    struct Nodeid
{
        int id2;			/* Secondary class identifier code */
        int size;			/* Per-instance structure size */
        Tree *(*mk)(Tree *, struct Nodeid *, va_list ap); /* Constructor */
        Tree *(*dup)(Tree *);	/* Duplicator */
        char *name;			/* Name used in show */
        void (*show)(Tree *, FILE *f, int flg, int lvl);	/* Show */
        int (*rm)(Tree *);	/* Destructor */
        void (*emit)(Tree *);	/* Emit */
        Tree *(*load)(struct Nodeid *);	/* Load */
        Tree *(*simp)(Tree *);	/* Simplify */
};

    /* Per-class structure.  There is only one instance of this structure for
        * the class, since this contains members which remain constant for each
        * object instance.
        *
        * The address of this structure is used as the primary run-time
        * identification code for the class.  To test the class type of an object
        * pointed to by x you usually say: x->String.id==Stringid,
        * x->Add.id==Addid, etc.
        *
        * The structure is defined as an array with a single element like this as
        * a trick to eliminate the need for '&' in the test above.  You can also
        * use the pointer dereference operator directly on the structure:
        * Nodeid->size.
        */

    /* Per-instance members.  These are the variable members of the class- they
        * can be different in each object instance (or just 'object'-
        * 'object instance' and 'object' can be used as synonyms).
        *
        * Object per-instance members are defined in a preprocessor definition so
        * that derived classes can inherit these members simply by naming the
        * preprocessor '#define' name in their per-instance structure.
        */

    #define NodeVARS \
        struct Nodeid *id;

    /* Per-instance structure.   The 'id' member (which points to the per-class
        * structure) is not defined in the preprocessor definition since it is
        * difference for each class.
        */

    struct Nodevars { NodeVARS };

    /* Virtual function dispatch macros.  These can be used on any tree node which
        * inherits the generic tree node 'Node'.
        */

    /* Generic constructor, calls the one in the Nodeid */
    Tree *mk(struct Nodeid *id, ...);

    /* Destroy and free object (not used in database manager) */

    #define rm(n) ((n)&&((Tree *)(n))->Node.id->rm(n))

    /* Duplicate object */

    #define dup(n) (((Tree *)(n))->Node.id->dup(n))

    /* Print object in readable form to a FILE */

    #define show(n,file,flg,lvl) (((Tree *)(n))->Node.id->show((n),(file),(flg),(lvl)))

    /* Emit object */
    
    #define emit(n) (((Tree *)(n))->Node.id->emit(n))

    /* Simplify object */

    #define simp(n) ((n)?((Tree *)(n))->Node.id->simp(n):0)

/* Integer constant */

    #define IntVARS \
        NodeVARS /* Inherit Node */ \
        int val; /* Value of integer constant */

    typedef struct Intvars Int;
    extern struct Nodeid Intid[];
    struct Intvars { IntVARS };

/* String constant tree node */

    #define StringVARS \
        NodeVARS \
        char *val; \
        int len; \

    typedef struct Stringvars String;
    extern struct Nodeid Stringid[];
    struct Stringvars { StringVARS };

/* Name of something (table or column) */

    #define NameVARS \
        NodeVARS \
        char *name; \

    typedef struct Namevars Name;
    extern struct Nodeid Nameid[];
    struct Namevars { NameVARS };

/* Symbol table entry reference */

#define SymbolVARS \
    NodeVARS \
    void *sym; \

    typedef struct Symbolvars Symbol;
    extern struct Nodeid Symbolid[];
    struct Symbolvars { SymbolVARS };

/* Section relative value */

#define LabelVARS \
    NodeVARS \
    unsigned long offset;	/* Offset from start of fragment */ \
    struct frag *frag;	/* Fragment we're relative to */ \
    struct section *sec;	/* Section we're relative to */ \

    typedef struct Labelvars Label;
    extern struct Nodeid Labelid[];
    struct Labelvars { LabelVARS };

/* Single operand operator */

    #define SingleVARS \
        NodeVARS \
        Tree *operand; \

    typedef struct Singlevars Single;
    struct Singlevars { SingleVARS };

/* Logical not */

    #define NotVARS \
        SingleVARS \

    typedef struct Notvars Not;
    extern struct Nodeid Notid[];
    struct Notvars { NotVARS };

/* Complement */

    #define ComVARS \
        SingleVARS \

    typedef struct Comvars Com;
    extern struct Nodeid Comid[];
    struct Comvars { ComVARS };

/* Negate */

    #define NegVARS \
        SingleVARS \

    typedef struct Negvars Neg;
    extern struct Nodeid Negid[];
    struct Negvars { NegVARS };

/* Double operand operator */

    #define DoubleVARS \
        NodeVARS \
        Tree *left; \
        Tree *right; \

    typedef struct Doublevars Double;
    struct Doublevars { DoubleVARS };

/* Add */

    #define AddVARS \
        DoubleVARS \

    typedef struct Addvars Add;
    extern struct Nodeid Addid[];
    struct Addvars { AddVARS };

/* Subtract */

    #define SubVARS \
        DoubleVARS \

    typedef struct Subvars Sub;
    extern struct Nodeid Subid[];
    struct Subvars { SubVARS };

/* Multiply */

    #define MulVARS \
        DoubleVARS \

    typedef struct Mulvars Mul;
    extern struct Nodeid Mulid[];
    struct Mulvars { MulVARS };

/* Divide */

    #define DivVARS \
        DoubleVARS \

    typedef struct Divvars Div;
    extern struct Nodeid Divid[];
    struct Divvars { DivVARS };

/* Modulus */

    #define ModVARS \
        DoubleVARS \

    typedef struct Modvars Mod;
    extern struct Nodeid Modid[];
    struct Modvars { ModVARS };

/* And */

    #define AndVARS \
        DoubleVARS \

    typedef struct Andvars And;
    extern struct Nodeid Andid[];
    struct Andvars { AndVARS };

/* Or */

    #define OrVARS \
        DoubleVARS \

    typedef struct Orvars Or;
    extern struct Nodeid Orid[];
    struct Orvars { OrVARS };

/* Xor */

    #define XorVARS \
        DoubleVARS \

    typedef struct Xorvars Xor;
    extern struct Nodeid Xorid[];
    struct Xorvars { XorVARS };

/* Equal */

    #define EqVARS \
        DoubleVARS \

    typedef struct Eqvars Eq;
    extern struct Nodeid Eqid[];
    struct Eqvars { EqVARS };

/* Not equal */

    #define NeVARS \
        DoubleVARS \

    typedef struct Nevars Ne;
    extern struct Nodeid Neid[];
    struct Nevars { NeVARS };

/* Greater than */

    #define GtVARS \
        DoubleVARS \

    typedef struct Gtvars Gt;
    extern struct Nodeid Gtid[];
    struct Gtvars { GtVARS };

/* Greater than or equal to */

    #define GeVARS \
        DoubleVARS \

    typedef struct Gevars Ge;
    extern struct Nodeid Geid[];
    struct Gevars { GeVARS };

/* Less than */

    #define LtVARS \
        DoubleVARS \

    typedef struct Ltvars Lt;
    extern struct Nodeid Ltid[];
    struct Ltvars { LtVARS };

/* Less than or equal to */

    #define LeVARS \
        DoubleVARS \

    typedef struct Levars Le;
    extern struct Nodeid Leid[];
    struct Levars { LeVARS };

/* Logical and */

    #define LandVARS \
        DoubleVARS \

    typedef struct Landvars Land;
    extern struct Nodeid Landid[];
    struct Landvars { LandVARS };

/* Logical or */

    #define LorVARS \
        DoubleVARS \

    typedef struct Lorvars Lor;
    extern struct Nodeid Lorid[];
    struct Lorvars { LorVARS };

/* Shift left */

    #define ShlVARS \
        DoubleVARS \

    typedef struct Shlvars Shl;
    extern struct Nodeid Shlid[];
    struct Shlvars { ShlVARS };

/* Shift right */

    #define ShrVARS \
        DoubleVARS \

    typedef struct Shrvars Shr;
    extern struct Nodeid Shrid[];
    struct Shrvars { ShrVARS };

/* Paren - left(right).  Could be a function call or a Select. */

    #define ParenVARS \
        DoubleVARS \

    typedef struct Parenvars Paren;
    extern struct Nodeid Parenid[];
    struct Parenvars { ParenVARS };

/* Three operand operator */

    #define TripleVARS \
        NodeVARS \
        Tree *test; \
        Tree *left; \
        Tree *right; \

    typedef struct Triplevars Triple;
    struct Triplevars { TripleVARS };

/* Mux */

    #define MuxVARS \
        TripleVARS \

    typedef struct Muxvars Mux;
    extern struct Nodeid Muxid[];
    struct Muxvars { MuxVARS };

/* 'Tree' is a union of all possible tree node classes and is used to make
    * generic tree node pointers 'Tree *'.  Technically we should use the base
    * class for tree nodes 'Node' for this purpose, but this union type ends
    * up being so much more convenient that it is used instead.  Frequently we
    * have to cast a base class pointer to a derived class and dereference the
    * pointer with a member access: ((Int *)((Iadd *)generic)->left)->val.  With
    * the union this operation is shorter and clearer: generic->Iadd.left->Int.val
    *
    * I would have liked to have made this a union of pointers so that the
    * above operations would look like generic.Iadd->left.Int->val (which I
    * think is even clearer), but this causes too many problems since many C
    * compilers do not allow you to cast a pointer to union.  Basically the
    * following does not work:
    *
    * union tree_pointers { struct Nodevars *Node; struct Intvars *Int; ... };
    * struct Nodevars y;
    * union tree_pointers x= &y; Error!
    * union tree_pointers x= (union tree_pointers)&y; Error! (but ok in GNU-C)
    *
    * Unfortunately centralized organizations like this do deviate from the
    * distributed object oriented ideal (meaning that when you derive a new
    * class, you have to remember to add it to this union).
    */

union tree
{
    struct Nodevars Node;
    struct Intvars Int;
    struct Stringvars String;
    struct Namevars Name;
    struct Symbolvars Symbol;
    struct Labelvars Label;
    struct Singlevars Single;
    struct Notvars Not;
    struct Comvars Com;
    struct Negvars Neg;
    struct Doublevars Double;
    struct Mulvars Mul;
    struct Divvars Div;
    struct Modvars Mod;
    struct Addvars Add;
    struct Subvars Sub;
    struct Andvars And;
    struct Orvars Or;
    struct Xorvars Xor;
    struct Eqvars Eq;
    struct Nevars Ne;
    struct Gtvars Gt;
    struct Ltvars Lt;
    struct Gevars Ge;
    struct Levars Le;
    struct Landvars Land;
    struct Lorvars Lor;
    struct Parenvars Paren;
    struct Shrvars Shr;
    struct Shlvars Shl;
    struct Triplevars Triple;
    struct Muxvars Mux;
};

/* Secondary class identifier codes.
    *
    * The primary method for identifying a class type at run time is to use the
    * address of the per-class structure as an identifying code.  Unfortunately
    * this address, although a constant, can not be used in the C-language
    * 'switch' statement.  So we provide a secondary code defined by the following
    * giant ugly 'enum' as a backup identifier code which can be used in 'switch'
    * statements.
    *
    * This is very stupid and is another centralized organization which
    * deviates from the distributed objected oriented ideal (as is the big Tree
    * union).  But we really want to be able to use the 'switch' statement.
    *
    * A primary code to secondary code translation can be performed by accessing
    * the 'id2' member of the per-class structure.
    */

#define iNODE 0x80
#define iINT 0x81
#define iSTRING 0x82
#define iNAME 0x83
#define iSYMBOL 0x84
#define iLABEL 0x85
#define iSINGLE 0x86
#define iNOT 0x87
#define iCOM 0x88
#define iNEG 0x89
#define iDOUBLE 0x8A
#define iMUL 0x8B
#define iDIV 0x8C
#define iMOD 0x8D
#define iADD 0x8E
#define iSUB 0x8F
#define iAND 0x90
#define iOR 0x91
#define iXOR 0x92
#define iEQ 0x93
#define iNE 0x94
#define iGT 0x95
#define iLT 0x96
#define iGE 0x97
#define iLE 0x98
#define iLAND 0x99
#define iLOR 0x9A
#define iSHR 0x9B
#define iSHL 0x9C
#define iPAREN 0x9D
#define iTRIPLE 0x9E
#define iMUX 0x9F

#define iMODULE 0xF0
#define iFIXUPS 0xF1
#define iXDEFS 0xF2
#define iSECTS 0xF3
#define iFRAG 0xF4
#define iSYMS 0xF5
#define iEND 0xF6

/* Secondary code to primary code translation table.  Boy is this stupid.
    * Anyway, to get the per-class structure address from a secondary code just
    * index this table.  Be sure to keep this table in the same order as the
    * above enum.
    */

extern struct Nodeid *idtable[];

Tree *interm(void);

void indent(FILE *file,int lvl);


#endif
