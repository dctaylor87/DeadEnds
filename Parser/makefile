.PRECIOUS: grammar.c

CC=clang
CFLAGS=-c -g -Wall
INCLUDES=-I. -I./Includes -I../DataTypes/Includes -I../Gedcom/Includes -I../Interpreter/Includes -I../Database/Includes -I../Utilities/Includes
AR=ar
ARFLAGS=-cr
OFILES=parse.o lexer.o grammar.o
LIBNAME=parser

$(LIBNAME)lib.a: $(OFILES)
	$(AR) $(ARFLAGS) $(LIBNAME)lib.a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<

%.c: %.y
	oyacc -d $<
	cp -f y.tab.c $@
