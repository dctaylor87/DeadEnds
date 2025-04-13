# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Interpreter library

CC=clang

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = \
	builtin.c \
	builtintable.c \
	evaluate.c \
	functable.c \
	functiontable.c \
	interp.c \
	intrpevent.c \
	intrpfamily.c \
	intrpgnode.c \
        intrpmath.c \
	intrpperson.c \
	intrpseq.c \
	intrpstring.c \
	pnode.c \
	pvalue.c \
	pvaluetable.c \
	sequence.c \
	symboltable.c \
	builtinlist.c \
	rassa.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/LLSupport/Includes

#AM_CFLAGS = -Wno-unused-but-set-variable -Wno-missing-braces
AM_CLFAGS = -fanalyzer
