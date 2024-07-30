# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# what are we going to build?
bin_PROGRAMS = runscript

runscript_SOURCES = runscript.c

runscript_LDADD = \
	../DeadEndsLib/Interp/libinterp.a \
	../DeadEndsLib/Parser/libparser.a \
	../DeadEndsLib/Operations/liboperations.a \
	../DeadEndsLib/Database/libdatabase.a \
	../DeadEndsLib/Validate/libvalidate.a \
	../DeadEndsLib/Gedcom/libgedcom.a \
	../DeadEndsLib/DataTypes/libdatatypes.a \
	../DeadEndsLib/Utils/libutils.a

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/Operations/Includes \
	-I$(top_srcdir)/DeadEndsLib/Validate/Includes
