CC=clang
CFLAGS=-c -g -Wall -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable
INCLUDES=-I./Includes -I../DataTypes/Includes -I../Gedcom/Includes -I../Utils/Includes -I../Validate/Includes
AR=ar
ARFLAGS=-cr
OFILES=database.o nameindex.o recordindex.o import.o removeops.o refnindex.o
LIBNAME=database

lib$(LIBNAME).a: $(OFILES)
	$(AR) $(ARFLAGS) lib$(LIBNAME).a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<

clean:
	rm -f *.o *.a
