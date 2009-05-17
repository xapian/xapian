## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

exampledatadir = $(docdir)/tcl8/examples
dist_exampledata_DATA = examples/simplesearch.tcl examples/simpleindex.tcl \
 examples/simpleexpand.tcl

docdatadir = $(docdir)/tcl8
dist_docdata_DATA = index.html
