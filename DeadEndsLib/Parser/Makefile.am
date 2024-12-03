# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Parser library

#.PRECIOUS: grammar.c

noinst_LIBRARIES = libparser.a

libparser_a_SOURCES = \
	parse.c \
	lexer.c \
	grammar.y

BUILT_SOURCES = grammar.h

YACC = byacc
AM_YFLAGS = -d

AM_CPPFLAGS = \
	-I$(top_builddir)/DeadEndsLib/Parser/ \
	-I$(top_builddir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes

AM_CFLAGS = -fanalyzer
