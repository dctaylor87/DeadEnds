# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds GEDCOM library

noinst_LIBRARIES = liboperations.a

liboperations_a_SOURCES = \
	addtofamily.c \
	createfamily.c \
	generatekey.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/Operations/Includes \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes

#AM_CFLAGS = -Wno-unused-but-set-variable
AM_CFLAGS = -fanalyzer
