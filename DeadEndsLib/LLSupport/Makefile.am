# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds LLSupport library

# OLD: datei.c, equaliso.c, ll-database.c, xreffile.c

noinst_LIBRARIES = libllsupport.a

libllsupport_a_SOURCES = \
	annotate.c \
	ask.c \
	brwslist.c \
	charmaps.c \
	charprops.c \
	choose.c \
	codesets.c \
	datei.c \
	dateprint.c \
	dispfmt.c \
	errlog.c \
	expandvars.c \
	gstrings.c \
	icvt.c \
	ll-addoperations.c \
	ll-indiseq.c \
	ll-node.c \
	ll-node-seq.c \
	ll-sequence.c \
	ll-str.c \
	llgettext.c \
	locales.c \
	norm_charmap.c \
	proptbls.c \
	remove.c \
	replace.c \
	rptui.c \
	sprintpic.c \
	strcvt.c \
	translat.c \
	valid.c \
	write.c \
	xlat.c \
	xref.c

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/DeadEndsLib/LLSupport/Includes \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/DeadEndsLib/Python/Includes \
	-I$(top_srcdir)/DeadEndsLib/UI/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Services/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/Validate/Includes

AM_CLFAGS = -fanalyzer

# Locale information for gettext
DEFS = -DDEADENDS -DLOCALEDIR=\"$(localedir)\" @DEFS@

