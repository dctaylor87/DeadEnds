# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# what are we going to build?
bin_PROGRAMS = hashtabletest stringtabletest

# what goes into hashtabletest:
hashtabletest_SOURCES = testhashtable.c

# what goes into stringtabletest:
stringtabletest_SOURCES = stringtabletest

# extra stuff for hashtabletest
hashtabletest_LDDADD = ../DataTypes/libdatatypes.a

# extra stuff for hashtabletest
stringtabletest_LDDADD = ../DataTypes/libdatatypes.a

# $(top_builddir)        for files in the build tree
# $(top_srcdir)          for files in the source tree

AM_CPPFLAGS = -I${top_srcdir}/DataTypes/Includes

# these dependencies are so that building just the tests plus whatever
# they depend upon will work

 ../DataTypes/libdatatypes.a:
	${MAKE} -C ../DataTypes libdatatypes.a
