## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include # -I$(top_srcdir)/testsuite

xapianincludedir = $(includedir)/xapian
omcompatincludedir = $(includedir)/om

xapianinclude_HEADERS = queryparser.h
omcompatinclude_HEADERS = omparsequery.h

lib_LTLIBRARIES = libomqueryparser.la
check_PROGRAMS = queryparsertest
TESTS = queryparsertest

libomqueryparser_la_SOURCES = queryparser.yy
queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = libomqueryparser.la ../libxapian.la
