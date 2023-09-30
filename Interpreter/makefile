CC=clang
CFLAGS=-c -g -Wall -Wno-unused-but-set-variable -Wno-missing-braces
INCLUDES=-I./Includes -I../Utilities/Includes -I../DataTypes/Includes -I../Gedcom/Includes -I../Database/Includes
AR=ar
ARFLAGS=-cr
OFILES= builtin.o builtintable.o evaluate.o functable.o functiontable.o interp.o intrpevent.o intrpfamily.o intrpgnode.o \
        intrpmath.o intrpperson.o intrpseq.o pnode.o pvalue.o pvaluetable.o sequence.o symboltable.o
LIBNAME=interpreter

$(LIBNAME)lib.a: $(OFILES)
	$(AR) $(ARFLAGS) $(LIBNAME)lib.a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
