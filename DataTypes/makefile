CC=clang
CFLAGS=-c -g -Wall
INCLUDES=-I./Includes -I../Utilities/Includes
AR=ar
ARFLAGS=-cr
OFILES=list.o hashtable.o sort.o set.o stringtable.o integertable.o

datatypelib.a: $(OFILES)
	$(AR) $(ARFLAGS) datatypeslib.a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
