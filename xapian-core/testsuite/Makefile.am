## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libtest.la

noinst_HEADERS = backendmanager.h testsuite.h testutils.h \
	allocdata.h alloccommon.h

EXTRA_DIST = malloccheck.c malloc_wrapper.sh

libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc \
		allocdata.cc alloccommon.c

libtest_la_LIBADD = -ldl

all-local: malloccheck.so

malloccheck.so: malloccheck.c alloccommon.lo
	$(LIBTOOL) $(CC) -shared $< alloccommon.lo -o $@ -ldl
