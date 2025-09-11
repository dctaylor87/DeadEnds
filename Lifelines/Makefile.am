# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# what are we going to build?
bin_PROGRAMS = llines llexec

# what goes into lines?
# export.c import.c

common_SOURCES = \
	environ.c \
	init.c \
	interp-prog.c \
	lines_usage.c \
	llabort.c \
	miscutls.c \
	parse-args.c \
	selectdb.c \
	signals.c \
	version.c

llines_SOURCES = add.c advedit.c askgedc.c askprogram.c \
	browse.c brwsmenu.c \
	cscurses.c delete.c dynmenu.c \
	edit.c interact.c \
	lbrowse.c listui.c loadsave.c \
	main.c menuset.c merge.c \
	newrecs.c pedigree.c \
	scan.c screen.c searchui.c show.c swap.c \
	tandem.c \
	opt-llines.c \
	$(common_SOURCES)

# what goes into llexec ?
llexec_SOURCES = llexec.c \
	llexec.h \
	opt-llexec.c \
	$(common_SOURCES)

PYTHON_LIBS = @PYTHON_LIBS@

llines_LDADD = @LIBPYLLINES@ \
	../DeadEndsLib/LLSupport/libllsupport.a \
	../DeadEndsLib/UI/libui.a \
	 ../DeadEndsLib/Parser/libparser.a \
	../DeadEndsLib/Interp/libinterp.a \
	../DeadEndsLib/Operations/liboperations.a \
	../DeadEndsLib/Database/libdatabase.a \
	../DeadEndsLib/Validate/libvalidate.a \
	../DeadEndsLib/Gedcom/libgedcom.a \
	../DeadEndsLib/DataTypes/libdatatypes.a ../DeadEndsLib/Utils/libutils.a

llexec_LDADD = @LIBPYLLINES@ \
	../DeadEndsLib/UI/libui.a \
	../DeadEndsLib/LLSupport/libllsupport.a \
	../DeadEndsLib/Parser/libparser.a \
	../DeadEndsLib/Interp/libinterp.a \
	../DeadEndsLib/Operations/liboperations.a \
	../DeadEndsLib/Database/libdatabase.a \
	../DeadEndsLib/Validate/libvalidate.a \
	../DeadEndsLib/Gedcom/libgedcom.a \
	../DeadEndsLib/DataTypes/libdatatypes.a ../DeadEndsLib/Utils/libutils.a

LIBS =	$(PYTHON_LIBS) @LIBINTL@ @LIBICONV@ @LIBS@

EXTRA_llines_DEPENDENCIES = $(LIBPYLLINES)
EXTRA_llexec_DEPENDENCIES = $(LIBPYLLINES)

# $(top_builddir)        for config.h
# $(top_srcdir)/...      for headers

# Flags needed to compile Python code
PYTHON_CPPFLAGS = @PYTHON_CPPFLAGS@
PYTHON_CFLAGS = @PYTHON_CFLAGS@

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/DeadEndsLib/LLSupport/Includes \
	-I$(top_srcdir)/DeadEndsLib/Python/Includes \
	-I$(top_srcdir)/DeadEndsLib/UI/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	$(PYTHON_CPPFLAGS)

AM_CLFAGS = -fanalyzer

# Locale information for gettext
DEFS = -DDEADENDS -DLOCALEDIR=\"$(localedir)\" -DSYS_CONF_DIR=\"$(sysconfdir)\" @DEFS@

EXTRA_DIST = llinesi.h screeni.h

# these dependencies are so that building parts will work

../DeadEndsLib/Interp/libinterp.a:
	$(MAKE) -C ../DeadEndsLib/Interp libinterp.a

../DeadEndsLib/Parser/libparser.a:
	$(MAKE) -C ../DeadEndsLib/Parser libparser.a

../DeadEndsLib/Database/libdatabase.a:
	$(MAKE) -C ../DeadEndsLib/Database libdatabase.a

../DeadEndsLib/Gedcom/libgedcom.a:
	$(MAKE) -C ../DeadEndsLib/Gedcom libgedcom.a

../DeadEndsLib/DataTypes/libdatatypes.a:
	$(MAKE) -C ../DeadEndsLib/DataTypes libdatatypes.a

../DeadEndsLib/Utils/libutils.a:
	$(MAKE) -C ../DeadEndsLib/Utils libutils.a
