bin_PROGRAMS = testprogram

testprogram_SOURCES = \
	test.c \
	testsequence.c \
	testgedcomstrings.c \
	testwritedatabase.c \
	importone.c \
	testgedpath.c

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/DeadEndsLib/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Operations/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/Validate/Includes

testprogram_LDADD =../DeadEndsLib/Interp/libinterp.a \
	../DeadEndsLib/Parser/libparser.a \
	../DeadEndsLib/Operations/liboperations.a \
	../DeadEndsLib/Database/libdatabase.a \
	../DeadEndsLib/Validate/libvalidate.a \
	../DeadEndsLib/Gedcom/libgedcom.a \
	../DeadEndsLib/DataTypes/libdatatypes.a \
	../DeadEndsLib/Utils/libutils.a

TEST_DIRS = \
	-DREPORTS_DIR=\"$(top_srcdir)/Reports/\" \
	-DSCRIPTS_DIR=\"$(top_srcdir)/Scripts\" \
	-DGEDFILES_DIR=\"$(top_srcdir)/Gedfiles\"

DEFS = $(TEST_DIRS) @DEFS@
