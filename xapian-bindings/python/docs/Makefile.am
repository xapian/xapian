## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

exampledatadir = $(docdir)/python/examples
dist_exampledata_DATA = examples/simplesearch.py examples/simpleindex.py \
    examples/simpleexpand.py examples/simplematchdecider.py

docdatadir = $(docdir)/python
dist_docdata_DATA = index.html
