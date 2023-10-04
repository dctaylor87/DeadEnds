# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Database library

noinst_LIBRARIES = libdatabase.a

libdatabase_a_SOURCES = \
	database.c \
	nameindex.c \
	recordindex.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Database/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Gedcom/Includes
