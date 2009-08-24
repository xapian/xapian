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
AUXFILES = style.css xapian-logo.png graded-background.png

# Files which should be put in the distribution by automake
EXTRA_DIST = $(MDDOCS) $(AUXFILES)

toolsdir=$(srcdir)/../../../xapian-maintainer-tools/html-tools
if !MAINTAINER_NO_DOCS
dist_doc_DATA = $(MDHTML) $(AUXFILES)

$(AUXFILES):
	cp $(toolsdir)/style.css style.css
	cp $(toolsdir)/xapian-logo.png xapian-logo.png
	cp $(toolsdir)/graded-background.png graded-background.png
endif

MAINTAINERCLEANFILES = $(MDHTML) $(AUXFILES)

if DOCUMENTATION_RULES
.md.html:
	$(toolsdir)/genpage -l $< docs-omega > $@
endif
