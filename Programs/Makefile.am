# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# what are we going to build?
bin_PROGRAMS = deadends testpython

# what goes into deadends

deadends_SOURCES = deadends.c de-scripts.c

testpython_SOURCES = testpython.c

# Ordering:
# Parser has to go before Interp
# Database has to go before GEDCOM
# Utils has to go before DataTypes due to searchIntegerTable,insertInIntegerTable
#
#
LDADD = ../DeadEndsLib/Python/libpyllines.a \
	../DeadEndsLib/LLSupport/libllsupport.a \
	../DeadEndsLib/UI/libui.a \
	../DeadEndsLib/Parser/libparser.a \
	../DeadEndsLib/Interp/libinterp.a \
	../DeadEndsLib/Operations/liboperations.a \
	../DeadEndsLib/Database/libdatabase.a \
	../DeadEndsLib/Validate/libvalidate.a \
	../DeadEndsLib/Gedcom/libgedcom.a \
	../DeadEndsLib/DataTypes/libdatatypes.a ../DeadEndsLib/Utils/libutils.a

#	../DeadEndsLib/Utils/libutils.a ../DeadEndsLib/DataTypes/libdatatypes.a

LIBS =  $(PYTHON_LIBS)

# $(top_builddir)        for files in the build tree
# $(top_srcdir)          for files in the source tree

# Flags needed to compile Python code
PYTHON_CPPFLAGS = @PYTHON_CPPFLAGS@
PYTHON_CFLAGS = @PYTHON_CFLAGS@

# Flags needed to link Python code
PYTHON_LIBS = @PYTHON_LIBS@

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/Programs/Includes \
	-I$(top_srcdir)/DeadEndsLib/Python/Includes \
	-I$(top_srcdir)/DeadEndsLib/LLSupport/Includes \
	-I$(top_srcdir)/DeadEndsLib/UI/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	$(PYTHON_CPPFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

#AM_CFLAGS = -Wno-unused-function
AM_CFLAGS = $(PYTHON_CFLAGS) -fanalyzer

# these dependencies are so that building just the tests plus whatever
# they depend upon will work.  It will only try to build the library
# if it does not exist.

../DeadEndsLib/DataTypes/libdatatypes.a:
	${MAKE} -C ../DeadEndsLib/DataTypes libdatatypes.a

../DeadEndsLib/Interp/libinterp.a:
	${MAKE} -C ../DeadEndsLib/Interp libinterp.a

../DeadEndsLib/Parser/libparser.a:
	${MAKE} -C ../DeadEndsLib/Parser libparser.a

../DeadEndsLib/Gedcom/libgedcom.a:
	${MAKE} -C ../DeadEndsLib/Gedcom libgedcom.a

../DeadEndsLib/Database/libdatabase.a:
	${MAKE} -C ../DeadEndsLib/Database libdatabase.a

../DeadEndsLib/Utils/libutils.a:
	${MAKE} -C ../DeadEndsLib/Utils libutils.a
