## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

RSTDOCS = \
	cgiparams.rst\
	encodings.rst\
	index.rst\
	newformat.rst\
	omegascript.rst\
	overview.rst\
	quickstart.rst\
	scriptindex.rst\
	termprefixes.rst

RSTHTML = $(RSTDOCS:.rst=.html)

# Files which should be put in the distribution by automake
EXTRA_DIST = $(RSTDOCS) $(GENERATED_RST)

if !MAINTAINER_NO_DOCS
dist_doc_DATA = $(RSTHTML)
endif

GENERATED_RST = inc/ignored.rst inc/mimetypes.rst
MAINTAINERCLEANFILES = $(RSTHTML) $(GENERATED_RST)

if DOCUMENTATION_RULES
overview.html: overview.rst $(GENERATED_RST)
	## docutils seems to lack any sort of "include path".
	perl -pe 'if (/^\.\.\s*include::\s*(\S+)/) {local $$/=undef;open I,"<",$$1 or open I,"<","$(srcdir)/$$1";$$_=<I>}' '$(srcdir)/overview.rst' |\
	    $(RST2HTML) --exit-status=warning - overview.html

.rst.html:
	$(RST2HTML) --exit-status=warning $< $@
endif
