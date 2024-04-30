# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Utils library

noinst_LIBRARIES = libutils.a

libutils_a_SOURCES = \
	errors.c \
	standard.c \
	unicode.c \
	path.c \
	utils.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes
