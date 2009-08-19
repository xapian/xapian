## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

exampledatadir = $(docdir)/csharp/examples
dist_exampledata_DATA =\
	examples/SimpleSearch.cs\
	examples/SimpleIndex.cs\
	examples/SimpleExpand.cs

docdatadir = $(docdir)/csharp
EXTRA_DIST = index.html

toolsdir=$(srcdir)/../../../xapian-maintainer-tools/html-tools

if !MAINTAINER_NO_DOCS
dist_docdata_DATA = index.html
endif

MAINTAINERCLEANFILES = index.html

if DOCUMENTATION_RULES
.md.html:
	$(toolsdir)/genpage -l $< docs-bindings > $@
endif
