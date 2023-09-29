CC=clang
CFLAGS=-c -g -Wall
INCLUDES=-I./Includes -I../DataTypes/Includes
AR=ar
ARFLAGS=-cr
OFILES=database.o nameindex.o recordindex.o

datatypelib.a: $(OFILES)
	$(AR) $(ARFLAGS) databaselib.a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
