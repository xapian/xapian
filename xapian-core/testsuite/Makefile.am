## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libtest.la liballoccommon.la

noinst_HEADERS = backendmanager.h testsuite.h testutils.h \
	allocdata.h alloccommon.h textfile_indexer.h index_utils.h

EXTRA_DIST = malloccheck.c malloc_wrapper.sh

if ENABLE_SHARED
libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc \
		allocdata.cc alloccommon.c textfile_indexer.cc index_utils.cc

liballoccommon_la_SOURCES = alloccommon.c

all-local: malloccheck.so alloccommon.so

CLEANFILES=malloccheck.so alloccommon.so

malloccheck.so: malloccheck.c
	$(LIBTOOL) $(CC) -shared $< -o $@ @DL_LIBS@ alloccommon.so

alloccommon.so: alloccommon.c
	$(LIBTOOL) $(CC) -shared $< -o $@
else
libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc \
		allocdata.cc alloccommon.c textfile_indexer.cc index_utils.cc \
		alloccommon.c
endif

libtest_la_LIBADD = @DL_LIBS@
