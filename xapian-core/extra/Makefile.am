## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include # -I$(top_srcdir)/testsuite

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

# 0:0:0 0.7.2 (and 0.7.1 and 0.7.0)
# 0:1:0 0.7.3 and 0.7.4
# 0:2:0 0.7.5
# 1:0:0 0.8.0
# LIBRARY_VERSION_INFO 0.8.1 and up
libxapianqueryparser_la_LDFLAGS = -version-info $(LIBRARY_VERSION_INFO)

libxapianqueryparser_la_SOURCES = $(YYSOURCES)

queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = libxapianqueryparser.la ../libxapian.la
