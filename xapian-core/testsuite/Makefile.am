## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libtest.la

noinst_HEADERS = backendmanager.h testsuite.h testutils.h \
	allocdata.h alloccommon.h textfile_indexer.h index_utils.h

EXTRA_DIST = malloccheck.c malloc_wrapper.sh

libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc \
		allocdata.cc alloccommon.c textfile_indexer.cc index_utils.cc

libtest_la_LIBADD = -ldl

if ENABLE_SHARED
all-local: malloccheck.so

CLEANFILES=malloccheck.so

malloccheck.so: malloccheck.c alloccommon.lo
	$(LIBTOOL) $(CC) -shared $< alloccommon.lo -o $@ -ldl
endif
