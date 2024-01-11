# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds LLSupport library

noinst_LIBRARIES = libllsupport.a

libllsupport_a_SOURCES = \
	charmaps.c \
	charprops.c \
	codesets.c \
	equaliso.c \
	llgettext.c \
	locales.c \
	messages.c \
	remove.c \
	replace.c \
	translat.c \
	valid.c \
	xlat.c \
	xreffile.c

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/LLSupport/Includes \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/Python/Includes \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Parser/Includes \
	-I$(top_srcdir)/Interp/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes

# Locale information for gettext
DEFS = -DDEADENDS -DLOCALEDIR=\"$(localedir)\" @DEFS@

