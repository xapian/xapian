## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

exampledatadir = $(docdir)/perl/examples
dist_exampledata_DATA = examples/full-indexer.pl examples/simpleexpand.pl \
	examples/simplematchdecider.pl examples/full-searcher.pl \
	examples/simpleindex.pl examples/simplesearch.pl

docdatadir = $(docdir)/perl
dist_docdata_DATA = index.html
