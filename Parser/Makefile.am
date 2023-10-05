# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Parser library

.PRECIOUS: grammar.c

noinst_LIBRARIES = libparser.a

libparser_a_SOURCES = \
	parse.c \
	lexer.c \
	grammar.y

YACC = byacc

AM_CPPFLAGS = \
	-I$(top_builddir)/Parser/Includes \
	-I$(top_srcdir)/Parser/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Interp/Includes \
	-I$(top_srcdir)/Database/Includes \
	-I$(top_srcdir)/Utils/Includes

AM_CLFAGS =
