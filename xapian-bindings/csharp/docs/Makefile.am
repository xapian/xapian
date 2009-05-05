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
dist_docdata_DATA = index.html
