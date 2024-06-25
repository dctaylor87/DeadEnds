# Makefile for the LifeLines report interpreter subsystem

noinst_LIBRARIES = libuicli.a libui.a
#noinst_LIBRARIES = libui.a

libui_core_SOURCES = ui.c print.c \
	curses-ui.c stdio-ui.c \
	uiioi.h

libui_a_SOURCES = $(libui_core_SOURCES)

libuicli_a_SOURCES = ui_cli.c $(libui_core_SOURCES)

# $(top_builddir)          for config.h
# $(top_srcdir)/...        for headers

# $(top_srcdir)/Lifelines/Includes is intentionally excluded

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/UI/Includes \
	-I$(top_srcdir)/LLSupport/Includes \
	-I$(top_srcdir)/Python/Includes \
	-I$(top_srcdir)/Utils/Includes \
	-I$(top_srcdir)/DataTypes/Includes \
	-I$(top_srcdir)/Parser/Includes \
	-I$(top_srcdir)/Interp/Includes \
	-I$(top_srcdir)/Gedcom/Includes \
	-I$(top_srcdir)/Database/Includes

DEFS = -DDEADENDS @DEFS@

AM_CLFAGS = -fanalyzer
