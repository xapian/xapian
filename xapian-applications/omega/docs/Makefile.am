## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

MDDOCS = \
	cgiparams.md\
	omegascript.md\
	overview.md\
	quickstart.md\
	scriptindex.md\
	termprefixes.md\
	index.md

MDHTML = $(MDDOCS:.md=.html)

# Files which should be put in the distribution by automake
EXTRA_DIST = $(MDDOCS)

toolsdir=$(srcdir)/../../../xapian-maintainer-tools/html-tools

if !MAINTAINER_NO_DOCS
dist_doc_DATA = $(MDHTML) \
	$(toolsdir)/style.css $(toolsdir)/xapian-logo.png \
	$(toolsdir)/graded-background.png
endif

MAINTAINERCLEANFILES = $(MDHTML)

if DOCUMENTATION_RULES
.md.html:
	$(toolsdir)/genpage -l $< docs-omega > $@
endif
