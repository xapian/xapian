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

# 0:0:0 0.7.2 (and anything before) 
# 0:1:0 0.7.3 and 0.7.4
libxapianqueryparser_la_LDFLAGS = -version-info 0:1:0

libxapianqueryparser_la_SOURCES = queryparser.yy

# For backward compatibility
# 0:0:0 0.7.3 (and anything before) 
# 0:1:0 0.7.4
libomqueryparser_la_LDFLAGS = -version-info 0:1:0

libomqueryparser_la_SOURCES = queryparser.yy

# FreeBSD's make insists on regenerating queryparser.cc for VPATH builds
# this stops that
queryparser.cc: $(srcdir)/queryparser.cc
	cp $(srcdir)/queryparser.cc .

queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = libomqueryparser.la ../libxapian.la
