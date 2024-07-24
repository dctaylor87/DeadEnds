# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds GEDCOM library

noinst_LIBRARIES = libgedcom.a

libgedcom_a_SOURCES = \
	date.c \
	gedcom.c \
	gnode.c \
	gnodelist.c \
	lineage.c \
	name.c \
	nodeutls.c \
	place.c \
	readnode.c \
	splitjoin.c \
	writenode.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes

#AM_CFLAGS = -Wno-unused-but-set-variable
AM_CFLAGS = -fanalyzer
