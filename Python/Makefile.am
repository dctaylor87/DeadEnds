# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the DeadEnds python support

# need to figure out how to specify the python dependency, for now...

noinst_LIBRARIES = libpyllines.a

libpyllines_a_SOURCES = \
	c-to-python.c \
	python-to-c.c python-to-c.h \
	database.c database.h \
	event.c \
	export.c \
	family.c family.h \
	iter.c \
	nodes.c \
	other.c \
	person.c person.h \
	porting.c \
	records.c \
	run.c \
	set.c set.h \
	source.c \
	types.c types.h \
	user.c

# need to figure out how to put in the dir for Python headers

DEFS = -DDEADENDS @DEFS@

# $(top_builddir)            for config.h
# $(top_srcdir)/.../Includes for headers

# Flags needed to compile Python code
PYTHON_CPPFLAGS = @PYTHON_CPPFLAGS@
PYTHON_CFLAGS = @PYTHON_CFLAGS@

# Flags needed to link Python code
PYTHON_LIBS = @PYTHON_LIBS@

# Currently we have no need of headers in
# $(top_srcdir)/Parser/Includes and $(top_srcdir)/Interp/Includes

AM_CPPFLAGS=-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/Python/Includes \
	-I$(top_srcdir)/Lifelines/Includes \
	-I$(top_srcdir)/Database/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Utils/Includes \
	$(PYTHON_CPPFLAGS)

AM_CFLAGS= $(PYTHON_CFLAGS) -Wno-cast-function-type

EXTRA_DIST =
