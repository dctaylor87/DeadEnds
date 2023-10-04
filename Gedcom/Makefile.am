# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds GEDCOM library

noinst_LIBRARIES = libgedcom.a

libgedcom_a_SOURCES = \
	gedcom.c \
	gnode.c \
	lineage.c \
	name.c \
	nodeutls.c \
	readnode.c \
	splitjoin.c \
	writenode.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Utilies/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Database/Includes

AM_CLFAGS = -Wno-unused-but-set-variable
