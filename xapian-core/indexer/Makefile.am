## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libindexer.la

libindexer_la_SOURCES = dummy.c

SUBDIRS = indexgraph
DIST_SUBDIRS = indexgraph

libindexer_la_LIBADD = indexgraph/libindexgraph.la

libxapianincludedir = $(includedir)/om
libxapianinclude_HEADERS = \
  omindexer.h omindexerbuilder.h omindexercommon.h omindexerdesc.h \
  omindexermessage.h omindexernode.h omnodedescriptor.h \
  ompaditerator.h omnodeinstanceiterator.h


if BUILD_INDEXER
INDEXERTESTS = indextest
INDEXERTESTS_RUN = run-indextest
else
INDEXERTESTS =
INDEXERTESTS_RUN =
endif

TESTS = $(INDEXERTESTS_RUN)

check_PROGRAMS = $(INDEXERTESTS)

indextest_SOURCES = indextest.cc
indextest_LDADD = ../testsuite/libtest.la ../libxapian.la

EXTRA_DIST = testdata/indextest_filereader.txt

run-indextest: indextest
	rm -f run-indextest
	$(LN_S) $(top_srcdir)/testsuite/malloc_wrapper.sh run-indextest
