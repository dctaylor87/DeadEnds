CC=gcc
CFLAGS=-g -c -Wall
INCLUDES= -I../Utils/Includes -I../DataTypes/Includes -I../Parser/Includes -I../Interp/Includes -I../Gedcom/Includes -I../Database/Includes
LIBLOCNS=-L../Utils/ -L../DataTypes/ -L../Parser/ -L../Interp -L../Gedcom -L../Database
LIBS=-lutils -lparser -ldatatypes -linterp -lgedcom -ldatabase

all: test

hashtabletest: testhashtable.o ../DataTypes/datatypeslib.a
	$(CC) -o hashtabletest testhashtable.o ../DataTypes/datatypeslib.a -lc

stringtabletest: teststringtable.o ../DataTypes/datatypeslib.a
	$(CC) -o stringtabletest teststringtable.o ../DataTypes/datatypeslib.a -lc

test: test.o
	$(CC) -o test test.o $(INCLUDES) $(LIBLOCNS) $(LIBS) -lc

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
