# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# not sure that this matters...
CC=clang

# Makefile for the DeadEnds Database library

noinst_LIBRARIES = libvalidate.a

libvalidate_a_SOURCES = \
	validate.c \
	valperson.c \
	valfamily.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Database/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Validate/Includes \
	-I$(top_srcdir)/Utils/Includes

#AM_CFLAGS = -Wno-unused-function -Wno-unused-but-set-variable
#AM_CFLAGS = -fanalyzer
