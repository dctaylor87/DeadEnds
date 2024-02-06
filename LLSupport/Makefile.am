# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds LLSupport library

noinst_LIBRARIES = libllsupport.a

libllsupport_a_SOURCES = \
	appendstr.c \
	brwslist.c \
	charmaps.c \
	charprops.c \
	codesets.c \
	equaliso.c \
	icvt.c \
	listener.c \
	ll-addoperations.c \
	llgettext.c \
	locales.c \
	messages.c \
	proptbls.c \
	refns.c \
	remove.c \
	replace.c \
	stdstrng.c \
	strapp.c \
	strcvt.c \
	strset.c \
	strutf8.c \
	strwhite.c \
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

