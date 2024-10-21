# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# what are we going to build?
bin_PROGRAMS = partition

partition_SOURCES = main.c connect.c

partition_LDADD = \
	../DeadEndsLib/Database/libdatabase.a \
	../DeadEndsLib/Validate/libvalidate.a \
	../DeadEndsLib/Gedcom/libgedcom.a \
	../DeadEndsLib/DataTypes/libdatatypes.a \
	../DeadEndsLib/Utils/libutils.a

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/Partition \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Validate/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/Operations/Includes

