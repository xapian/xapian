## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I${top_builddir}/include -I$(top_srcdir)/common -I$(top_srcdir)/api

noinst_HEADERS = queryparser_internal.h queryparser_token.h utf8itor.h tclUniData.h

noinst_LTLIBRARIES = libqueryparser.la

lemon_built_sources = queryparser_internal.cc queryparser_token.h
EXTRA_DIST = lemon.c queryparser.lemony queryparser.lt $(lemon_built_sources)

if MAINTAINER_MODE
lemon: lemon.c
	$(CC_FOR_BUILD) -o lemon $(srcdir)/lemon.c

# Lemon carefully avoids touching queryparser_token.h if it hasn't changed,
# but only the generated queryparser_internal.cc depends on it, so it's
# better to touch it so we can have a dependency to generate it.
queryparser_internal.cc queryparser_token.h: queryparser.lemony queryparser.lt lemon
	./lemon -q -oqueryparser_internal.cc -hqueryparser_token.h $(srcdir)/queryparser.lemony && touch queryparser_token.h

BUILT_SOURCES = $(lemon_built_sources)
MAINTAINERCLEANFILES = lemon $(lemon_built_sources)
endif

libqueryparser_la_SOURCES = queryparser.cc queryparser_internal.cc utf8itor.cc tclUniData.cc
