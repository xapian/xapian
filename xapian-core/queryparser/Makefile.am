## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include -I$(top_srcdir)/common -I$(top_srcdir)/api

noinst_HEADERS = accentnormalisingitor.h symboltab.h queryparser_internal.h \
	queryparser_token.h

noinst_LTLIBRARIES = libqueryparser.la

check_PROGRAMS = queryparsertest
TESTS = queryparsertest$(EXEEXT)

lemon_built_sources = queryparser_internal.cc queryparser_token.h
EXTRA_DIST = lemon.c queryparser.lemony queryparser.lt $(lemon_built_sources)

if MAINTAINER_MODE
lemon: lemon.c
	$(CC_FOR_BUILD) -o lemon $(srcdir)/lemon.c

queryparser_internal.cc queryparser_token.h: queryparser.lemony queryparser.lt lemon
	./lemon -q -oqueryparser_internal.cc -hqueryparser_token.h $(srcdir)/queryparser.lemony

BUILT_SOURCES = $(lemon_built_sources)
MAINTAINERCLEANFILES = lemon $(lemon_built_sources)
endif

libqueryparser_la_SOURCES = queryparser.cc queryparser_internal.cc

queryparsertest_SOURCES = queryparsertest.cc
queryparsertest_LDADD = @ldflags@ ../libxapian.la
