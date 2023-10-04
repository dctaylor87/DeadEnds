# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Interpreter library

noinst_LIBRARIES = libinterpreter.a

libinterpreter_a_SOURCES = \
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
	pnode.c \
	pvalue.c \
	pvaluetable.c \
	sequence.c \
	symboltable.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Interpreter/Includes \
	-I$(top_srcdir)/Utilities/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes

AM_CLFAGS = -Wno-unused-but-set-variable -Wno-missing-braces
