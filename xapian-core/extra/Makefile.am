## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include # -I$(top_srcdir)/testsuite

lib_LTLIBRARIES = libomqueryparser.la
libomqueryparserincludedir = $(includedir)/om
libomqueryparserinclude_HEADERS = omparsequery.h

check_PROGRAMS = parsequerytest
TESTS = parsequerytest

libomqueryparser_la_SOURCES = parsequery.yy
parsequerytest_SOURCES = parsequerytest.cc
parsequerytest_LDADD = libomqueryparser.la ../libxapian.la
