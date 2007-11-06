## Process this file with automake to produce Makefile.in

RSTDOCS = \
	cgiparams.rst\
	omegascript.rst\
	overview.rst\
	quickstart.rst\
	scriptindex.rst\
	termprefixes.rst

RSTHTML = $(RSTDOCS:.rst=.html)

# Files which should be put in the distribution by automake
EXTRA_DIST = $(RSTDOCS)

if !MAINTAINER_NO_DOCS
dist_doc_DATA = $(RSTHTML)
endif

MAINTAINERCLEANFILES = $(RSTHTML)

if DOCUMENTATION_RULES
.rst.html:
	$(RST2HTML) --exit-status=warning $< $@
endif
