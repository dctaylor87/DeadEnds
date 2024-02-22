# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# what are we going to build?
bin_PROGRAMS = llines

# what goes into lines?
# export.c import.c

llines_SOURCES = add.c advedit.c ask.c askgedc.c askprogram.c \
	browse.c brwsmenu.c \
	choose.c cscurses.c delete.c dynmenu.c \
	edit.c interact.c \
	lbrowse.c lines_usage.c listui.c \
	main.c menuset.c merge.c miscutls.c \
	newrecs.c pedigree.c \
	scan.c screen.c searchui.c selectdb.c show.c swap.c \
	tandem.c

PYTHON_LIBS = @PYTHON_LIBS@

LDADD = ../Python/libpyllines.a \
	../LLSupport/libllsupport.a \
	../Parser/libparser.a ../Interp/libinterp.a \
	../UI/libui.a \
	../Database/libdatabase.a ../Gedcom/libgedcom.a \
	../Validate/libvalidate.a \
	../DataTypes/libdatatypes.a ../Utils/libutils.a

LIBS =	$(PYTHON_LIBS) @LIBINTL@ @LIBICONV@ @LIBS@

EXTRA_llines_DEPENDENCIES = $(LIBPYLLINES)

# $(top_builddir)        for config.h
# $(top_srcdir)/src/hdrs for headers

# Flags needed to compile Python code
PYTHON_CPPFLAGS = @PYTHON_CPPFLAGS@
PYTHON_CFLAGS = @PYTHON_CFLAGS@

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/LLSupport/Includes \
	-I$(top_srcdir)/Python/Includes \
	-I$(top_srcdir)/UI/Includes \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Parser/Includes \
	-I$(top_srcdir)/Interp/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes \
	$(PYTHON_CPPFLAGS)

# Locale information for gettext
DEFS = -DDEADENDS -DLOCALEDIR=\"$(localedir)\" @DEFS@

EXTRA_DIST = llinesi.h screeni.h

# these dependencies are so that building parts will work

../Interp/libinterp.a:
	$(MAKE) -C ../Interp libinterp.a

../Parser/libparser.a:
	$(MAKE) -C ../Parser libparser.a

../Database/libdatabase.a:
	$(MAKE) -C ../Database libdatabase.a

../Gedcom/libgedcom.a:
	$(MAKE) -C ../Gedcom libgedcom.a

../DataTypes/libdatatypes.a:
	$(MAKE) -C ../DataTypes libdatatypes.a

../Utils/libutils.a:
	$(MAKE) -C ../Utils libutils.a
