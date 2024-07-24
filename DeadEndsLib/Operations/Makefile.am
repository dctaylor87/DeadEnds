# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds GEDCOM library

noinst_LIBRARIES = liboperations.a

liboperations_a_SOURCES = \
	addtofamily.c \
	createfamily.c \
	generatekey.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/DeadEndsLib/Operations/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes

#AM_CFLAGS = -Wno-unused-but-set-variable
AM_CFLAGS = -fanalyzer
