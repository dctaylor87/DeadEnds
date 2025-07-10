# This makefile creates the docs/manual directory

###################
# Manuals
###################

MANUALS_en =	ll-python-api.pdf \
		ll-python-internals.pdf \
		de-libraries.pdf

MANUALS_sv =
MANUALS =	$(MANUALS_en) $(MANUALS_sv)

MANUALSSRC_en = ll-python-api.texi \
		ll-python-internals.texi \
		de-libraries.texi

info_TEXINFOS = ll-python-api.texi ll-python-internals.texi

MANUALSSRC_sv =

MANUALSSRC =	$(MANUALSSRC_en) $(MANUALSSRC_sv)

IMAGES =	listops.jpg

MANUALS_static = index.html \
		 $(IMAGES)

###################
# Rules
###################

# Install static files and generated manuals, but only when docs are built
if BUILD_DOCS
#doc_DATA = $(MANUALS_static) $(MANUALS)
doc_DATA = $(MANUALS)
else
doc_DATA =
endif

# We distribute the above, plus sources.
EXTRA_DIST = $(doc_DATA) $(MANUALSSRC)

# By default, documentation cannot be cleaned.
CLEANFILES =

# When using --with-docs, provide the power to destroy and the rules to recreate.
if BUILD_DOCS
CLEANFILES += $(MANUALS)

.xml.pdf:
	@TOOL_DBLATEX@ $^

.xml.html:
	@TOOL_XMLTO@ --skip-validation xhtml-nochunks $^

.xml.txt:
	@TOOL_XMLTO@ --skip-validation txt $^

endif
