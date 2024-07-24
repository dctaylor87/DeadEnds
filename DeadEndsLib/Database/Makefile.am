# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# not sure that this matters...
#CC=clang

# Makefile for the DeadEnds Database library

noinst_LIBRARIES = libdatabase.a

libdatabase_a_SOURCES = \
	database.c \
	nameindex.c \
	recordindex.c \
	import.c \
	addops.c \
	removeops.c \
	refnindex.c \
	rootlist.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Validate/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes

#AM_CFLAGS = -Wno-unused-function -Wno-unused-but-set-variable
AM_CFLAGS = -fanalyzer

DEFS = -DDEADENDS @DEFS@
