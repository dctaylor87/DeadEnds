# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds LLSupport library

noinst_LIBRARIES = libllsupport.a

libllsupport_a_SOURCES = \
	appendstr.c \
	ask.c \
	brwslist.c \
	charmaps.c \
	charprops.c \
	choose.c \
	codesets.c \
	equaliso.c \
	expandvars.c \
	fpattern.c \
	gstrings.c \
	icvt.c \
	listener.c \
	ll-addoperations.c \
	ll-database.c \
	ll-node.c \
	ll-node-seq.c \
	ll-sequence.c \
	ll-str.c \
	ll-temp-nodes.c \
	llgettext.c \
	lloptions.c \
	locales.c \
	messages.c \
	norm_charmap.c \
	proptbls.c \
	refns.c \
	remove.c \
	replace.c \
	sprintpic.c \
	stdstrng.c \
	strapp.c \
	strcvt.c \
	strset.c \
	strutf8.c \
	strwhite.c \
	translat.c \
	valid.c \
	write.c \
	xlat.c \
	xref.c \
	xreffile.c

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/LLSupport/Includes \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/Python/Includes \
	-I$(top_srcdir)/UI/Includes \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Parser/Includes \
	-I$(top_srcdir)/Interp/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes

AM_CLFAGS = -fanalyzer

# Locale information for gettext
DEFS = -DDEADENDS -DLOCALEDIR=\"$(localedir)\" @DEFS@

