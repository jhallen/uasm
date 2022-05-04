CC = cc
CFLAGS = -g

all : uasm ulink

uasm : uasm.o strlist.o syntax.o direct.o listing.o parse.o tree.o expr.o frag.o macro.o symtab.o output.o error.o input.o hash.o
	$(CC) -o uasm -g uasm.o strlist.o syntax.o direct.o listing.o parse.o tree.o expr.o frag.o macro.o symtab.o output.o error.o input.o hash.o

ulink : ulink.o tree.o frag.o symtab.o input.o output.o library.o error.o hash.o
	$(CC) -o ulink -g ulink.o tree.o frag.o symtab.o input.o output.o library.o error.o hash.o

clean :
	rm -f *.o
