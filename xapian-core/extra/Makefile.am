## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include # -I$(top_srcdir)/testsuite

noinst_HEADERS = symboltab.h

xapianincludedir = $(includedir)/xapian
omcompatincludedir = $(includedir)/om

xapianinclude_HEADERS = xapian/queryparser.h
omcompatinclude_HEADERS = omparsequery.h

lib_LTLIBRARIES = libxapianqueryparser.la libomqueryparser.la
check_PROGRAMS = queryparsertest
TESTS = queryparsertest$(EXEEXT)

EXTRA_DIST = queryparser.yy

# In non-maintainer-mode, depend on the generated C++ source
if MAINTAINER_MODE
YYSOURCES = queryparser.yy
else
YYSOURCES = queryparser.cc
endif

# 0:0:0 0.7.2 (and anything before) 
# 0:1:0 0.7.3 and 0.7.4
# 0:2:0 0.7.5
# 1:0:0 0.8.0
libxapianqueryparser_la_LDFLAGS = -version-info 1:0:0

libxapianqueryparser_la_SOURCES = $(YYSOURCES)

# For backward compatibility
# 0:0:0 0.7.3 (and anything before) 
# 0:1:0 0.7.4
# 0:2:0 0.7.5
# 1:0:0 0.8.0
libomqueryparser_la_LDFLAGS = -version-info 1:0:0

libomqueryparser_la_SOURCES = $(YYSOURCES)

queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = libomqueryparser.la ../libxapian.la
