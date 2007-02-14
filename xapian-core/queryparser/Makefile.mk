noinst_HEADERS +=\
	queryparser/queryparser_internal.h\
	queryparser/queryparser_token.h\
	queryparser/tclUniData.h\
	queryparser/utf8itor.h

lemon_built_sources =\
	queryparser/queryparser_internal.cc\
	queryparser/queryparser_token.h

EXTRA_DIST += $(lemon_built_sources)\
	queryparser/dir_contents\
	queryparser/Makefile\
	queryparser/lemon.c\
	queryparser/queryparser.lemony\
	queryparser/queryparser.lt

if MAINTAINER_MODE
queryparser/lemon: queryparser/lemon.c
	$(CC_FOR_BUILD) -o queryparser/lemon $(srcdir)/queryparser/lemon.c

# Lemon carefully avoids touching queryparser_token.h if it hasn't changed,
# but only the generated file queryparser_internal.cc depends
# on it, so it's better to touch it so we can have a
# dependency to generate it.
queryparser/queryparser_internal.cc queryparser/queryparser_token.h: queryparser/queryparser.lemony queryparser/queryparser.lt queryparser/lemon
	queryparser/lemon -q -oqueryparser/queryparser_internal.cc -hqueryparser/queryparser_token.h $(srcdir)/queryparser/queryparser.lemony && touch queryparser/queryparser_token.h

BUILT_SOURCES += $(lemon_built_sources)
CLEANFILES += $(lemon_built_sources)\
	queryparser/lemon
else
MAINTAINERCLEANFILES += $(lemon_built_sources)\
	queryparser/lemon
endif

libxapian_la_SOURCES +=\
	queryparser/queryparser.cc\
	queryparser/queryparser_internal.cc\
	queryparser/tclUniData.cc\
	queryparser/utf8itor.cc
