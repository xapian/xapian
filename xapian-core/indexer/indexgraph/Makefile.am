## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = indexergraph.h omindexernode.h testnodes.h deleter_map.h

noinst_LTLIBRARIES = libindexgraph.la

noinst_PROGRAMS = nodetest labs_example

nodetest_SOURCES = nodetest.cc

nodetest_LDADD = libindexgraph.la ../../libomus.la

labs_example_SOURCES = labs_example.cc

labs_example_LDADD = libindexgraph.la ../../libomus.la

libindexgraph_la_SOURCES = indexergraph.cc omindexernode.cc

indexergraph.lo: indexergraph.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

omindexernode.lo: omindexernode.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

nodetest.lo: nodetest.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

nodetest.o: nodetest.cc
	$(CXXCOMPILE) @LIBXML_CFLAGS@ -c $<

labs_example.lo: labs_example.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

labs_example.o: labs_example.cc
	$(CXXCOMPILE) @LIBXML_CFLAGS@ -c $<

libindexgraph_la_LIBADD = @LIBXML_LIBS@
