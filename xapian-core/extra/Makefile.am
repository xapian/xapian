## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include -I$(top_srcdir)/common # -I$(top_srcdir)/testsuite

noinst_HEADERS = symboltab.h

xapianincludedir = $(includedir)/xapian

xapianinclude_HEADERS = xapian/queryparser.h

lib_LTLIBRARIES = libxapianqueryparser.la
check_PROGRAMS = queryparsertest
TESTS = queryparsertest$(EXEEXT)

EXTRA_DIST = queryparser.yy

# In non-maintainer-mode, depend on the generated C++ source
if MAINTAINER_MODE
YYSOURCES = queryparser.yy
else
YYSOURCES = queryparser.cc
endif

libxapianqueryparser_la_LDFLAGS = -version-info $(LIBRARY_VERSION_INFO)

libxapianqueryparser_la_SOURCES = $(YYSOURCES)

queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = libxapianqueryparser.la ../libxapian.la
