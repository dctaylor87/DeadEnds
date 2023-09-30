CC=gcc
CFLAGS=-g -c -Wall
INCLUDES= -I../Includes

all: hashtabletest stringtabletest

hashtabletest: testhashtable.o ../DataTypes/datatypeslib.a
	$(CC) -o hashtabletest testhashtable.o ../DataTypes/datatypeslib.a -lc

stringtabletest: teststringtable.o ../DataTypes/datatypeslib.a
	$(CC) -o stringtabletest teststringtable.o ../DataTypes/datatypeslib.a -lc

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
