# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Utils library

noinst_LIBRARIES = libutils.a

libutils_a_SOURCES = \
	appendstr.c \
	errors.c \
	fpattern.c \
	standard.c \
	stdstrng.c \
	strapp.c \
	strset.c \
	strutf8.c \
	strwhite.c \
	unicode.c \
	path.c \
	utils.c \
	file.c

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes

AM_CLFAGS = -fanalyzer
