## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include -I$(top_srcdir)/common -I$(top_srcdir)/api

noinst_HEADERS = accentnormalisingitor.h symboltab.h queryparser.h

xapianincludedir = $(includedir)/xapian

xapianinclude_HEADERS = xapian/queryparser.h

lib_LTLIBRARIES = libxapianqueryparser.la
check_PROGRAMS = queryparsertest
TESTS = queryparsertest$(EXEEXT)

EXTRA_DIST = queryparser.lemony queryparser_internal.cc lemon.c queryparser.lt

if MAINTAINER_MODE
lemon: lemon.c
	$(CC_FOR_BUILD) -o lemon $(srcdir)/lemon.c

queryparser_internal.cc queryparser.h: queryparser.lemony queryparser.lt lemon
	./lemon $(srcdir)/queryparser.lemony
	mv -f queryparser.c queryparser_internal.cc

MAINTAINERCLEANFILES = queryparser_internal.cc queryparser.h queryparser.c\
 queryparser.out lemon
endif

libxapianqueryparser_la_LDFLAGS = -no-undefined -version-info $(LIBRARY_VERSION_INFO)
libxapianqueryparser_la_LIBADD = @ldflags@ ../libxapian.la

libxapianqueryparser_la_SOURCES = queryparser.cc queryparser_internal.cc

queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = libxapianqueryparser.la ../libxapian.la
