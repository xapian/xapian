## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libtest.la

noinst_HEADERS = backendmanager.h testsuite.h testutils.h \
	textfile_indexer.h index_utils.h getopt.h

libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc \
	textfile_indexer.cc index_utils.cc \
	getopt.c getopt1.c

libtest_la_LIBADD = @DL_LIBS@
