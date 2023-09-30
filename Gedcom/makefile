CC=clang
CFLAGS=-c -g -Wall -Wno-unused-but-set-variable
INCLUDES=-I./Includes -I../Utilities/Includes -I../DataTypes/Includes -I../Database/Includes
AR=ar
ARFLAGS=-cr
OFILES=gedcom.o gnode.o lineage.o name.o nodeutls.o readnode.o splitjoin.o writenode.o
LIBNAME=gedcom

$(LIBNAME)lib.a: $(OFILES)
	$(AR) $(ARFLAGS) $(LIBNAME)lib.a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
