## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libindexer.la

libindexer_la_SOURCES = dummy.c

SUBDIRS = indexgraph
DIST_SUBDIRS = indexgraph

libindexer_la_LIBADD = indexgraph/libindexgraph.la
