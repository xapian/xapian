## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = index_utils.h

noinst_LTLIBRARIES = libindexer.la

libindexer_la_SOURCES = textfile_indexer.cc index_utils.cc query_parser.cc
