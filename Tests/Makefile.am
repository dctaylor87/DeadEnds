# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# not sure that this matters...
#CC=clang

# what are we going to build?
bin_PROGRAMS = test hashtabletest stringtabletest testpython

# what goes into hashtabletest, stringtabletest, test:

hashtabletest_SOURCES = testhashtable.c

stringtabletest_SOURCES = teststringtable.c

test_SOURCES = test.c

testpython_SOURCES = testpython.c

# extra stuff for hashtabletest, hashtabletest, test

# Ordering:
# Parser has to go before Interp
# Database has to go before GEDCOM
# Utils has to go before DataTypes due to searchIntegerTable,insertInIntegerTable
# 
# 
LDADD =  ../Python/libpyllines.a \
	../Parser/libparser.a ../Interp/libinterp.a \
	../Database/libdatabase.a ../Gedcom/libgedcom.a \
	../DataTypes/libdatatypes.a ../Utils/libutils.a

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
	-I$(top_srcdir)/Python/Includes \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Parser/Includes \
	-I$(top_srcdir)/Interp/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes \
	$(PYTHON_CPPFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

#AM_CFLAGS = -Wno-unused-function
AM_CFLAGS = $(PYTHON_CFLAGS)

# these dependencies are so that building just the tests plus whatever
# they depend upon will work.  It will only try to build the library
# if it does not exist.

../DataTypes/libdatatypes.a:
	${MAKE} -C ../DataTypes libdatatypes.a

../Interp/libinterp.a:
	${MAKE} -C ../Interp libinterp.a

../Parser/libparser.a:
	${MAKE} -C ../Parser libparser.a

../Gedcom/libgedcom.a:
	${MAKE} -C ../Gedcom libgedcom.a

../Database/libdatabase.a:
	${MAKE} -C ../Database libdatabase.a

../Utils/libutils.a:
	${MAKE} -C ../Utils libutils.a
