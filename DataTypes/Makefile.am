# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds DataTypes library

noinst_LIBRARIES = libdatatypes.a

libdatatypes_a_SOURCES = \
	list.c \
	hashtable.c \
	sort.c \
	set.c \
	stringtable.c \
	integertable.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Utils/Includes
