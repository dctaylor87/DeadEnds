# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds Services library

noinst_LIBRARIES = libservices.a

# this list will likely grow over time
libservices_a_SOURCES = \
	listener.c \
	lloptions.c

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/DeadEndsLib/Services/Includes \
	-I$(top_srcdir)/DeadEndsLib/LLSupport/Includes \
	-I$(top_srcdir)/DeadEndsLib/Python/Includes \
	-I$(top_srcdir)/DeadEndsLib/UI/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/Validate/Includes

AM_CLFAGS = -fanalyzer

# Locale information for gettext
DEFS = -DDEADENDS -DLOCALEDIR=\"$(localedir)\" @DEFS@

