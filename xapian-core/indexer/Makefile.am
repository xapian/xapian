## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = index_utils.h

noinst_LTLIBRARIES = libindexer.la

libindexer_la_SOURCES = textfile_indexer.cc index_utils.cc

if HAVE_LIBXML
BUILDSUBDIR_INDEXGRAPH = indexgraph
LIBADD_INDEXGRAPH = indexgraph/libindexgraph.la
else
BUILDSUBDIR_INDEXGRAPH =
LIBADD_INDEXGRAPH =
endif

SUBDIRS = $(BUILDSUBDIR_INDEXGRAPH)
DIST_SUBDIRS = indexgraph

libindexer_la_LIBADD = $(LIBADD_INDEXGRAPH)
