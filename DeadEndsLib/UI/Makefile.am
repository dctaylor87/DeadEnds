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
	-I$(top_srcdir)/DeadEndsLib/UI/Includes \
	-I$(top_srcdir)/DeadEndsLib/LLSupport/Includes \
	-I$(top_srcdir)/DeadEndsLib/Python/Includes \
	-I$(top_srcdir)/DeadEndsLib/Utils/Includes \
	-I$(top_srcdir)/DeadEndsLib/DataTypes/Includes \
	-I$(top_srcdir)/DeadEndsLib/Parser/Includes \
	-I$(top_srcdir)/DeadEndsLib/Interp/Includes \
	-I$(top_srcdir)/DeadEndsLib/Gedcom/Includes \
	-I$(top_srcdir)/DeadEndsLib/Database/Includes

DEFS = -DDEADENDS @DEFS@

AM_CLFAGS = -fanalyzer
