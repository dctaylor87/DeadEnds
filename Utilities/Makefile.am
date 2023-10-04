# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Utilities library

noinst_LIBRARIES = libutilities.a

libutilities_a_SOURCES = \
	data.c \
	errors.c \
	standard.c \
	unicode.c \
	path.c \
	utils.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Utilities/Includes \
	-I$(top_srcdir)/DataTypes/Includes
