CC=clang
CFLAGS=-c -g -Wall
INCLUDES=-I./Includes -I../DataTypes/Includes -I../Gedcom/Includes
AR=ar
ARFLAGS=-cr
OFILES=database.o nameindex.o recordindex.o
LIBNAME=database

lib$(LIBNAME).a: $(OFILES)
	$(AR) $(ARFLAGS) lib$(LIBNAME).a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
