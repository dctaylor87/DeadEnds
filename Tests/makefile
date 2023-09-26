CC=gcc
CFLAGS=-g -c -Wall
INCLUDES= -I../Includes

all: hashtabletest
hashtabletest: testhashtable.o
	$(CC) -o hashtabletest testhashtable.o ../DataTypes/datatypeslib.a -lc

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<
