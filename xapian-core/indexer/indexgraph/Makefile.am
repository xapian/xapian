## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = indexergraph.h omindexernode.h testnodes.h deleter_map.h

noinst_LTLIBRARIES = libindexgraph.la

#noinst_PROGRAMS = nodetest
#
#nodetest_SOURCES = nodetest.cc
#
#nodetest_LDADD = libindexgraph.la ../../libomus.la

INDEXERNODES = omstemmernode.cc

libindexgraph_la_SOURCES = indexergraph.cc omindexernode.cc \
	do_registration.cc $(INDEXERNODES)

omstemmernode.lo: omstemmernode.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

indexergraph.lo: indexergraph.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

omindexernode.lo: omindexernode.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

nodetest.lo: nodetest.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

nodetest.o: nodetest.cc
	$(CXXCOMPILE) @LIBXML_CFLAGS@ -c $<

do_registration.cc: $(INDEXERNODES) make_register.pl
	srcdir=$(srcdir) ./make_register.pl $(INDEXERNODES) > $@

do_registration.lo: do_registration.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

libindexgraph_la_LIBADD = @LIBXML_LIBS@
