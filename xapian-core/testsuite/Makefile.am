## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libtest.la

noinst_HEADERS = backendmanager.h testsuite.h testutils.h allocdata.h

EXTRA_DIST = malloccheck.c

libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc

all-local: malloccheck.so

malloccheck.so: malloccheck.c
	$(LIBTOOL) $(CC) -shared $< -o $@ -ldl
