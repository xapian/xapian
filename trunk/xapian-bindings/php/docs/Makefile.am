## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

exampledatadir = $(docdir)/php/examples
dist_exampledata_DATA =\
	examples/simplesearch.php5\
       	examples/simpleindex.php5\
       	examples/simpleexpand.php5

docdatadir = $(docdir)/php
dist_docdata_DATA = index.html
