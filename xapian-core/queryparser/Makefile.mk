if VPATH_BUILD
# We need this so that generated sources can find non-generated headers in a
# VPATH build from git.
AM_CPPFLAGS += -I$(top_srcdir)/queryparser
endif

noinst_HEADERS +=\
	queryparser/cjk-tokenizer.h\
	queryparser/queryparser_internal.h\
	queryparser/queryparser_token.h\
	queryparser/termgenerator_internal.h

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

queryparser/queryparser_internal.cc queryparser/queryparser_token.h: queryparser/queryparser_internal.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf queryparser/queryparser_internal.lock queryparser/queryparser_internal.stamp' 1 2 13 15; \
	  if mkdir queryparser/queryparser_internal.lock 2>/dev/null; then \
	    rm -f queryparser/queryparser_internal.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) queryparser/queryparser_internal.stamp; \
	    rmdir queryparser/queryparser_internal.lock; \
	  else \
	    while test -d queryparser/queryparser_internal.lock; do sleep 1; done; \
	    test -f queryparser/queryparser_internal.stamp; exit $$?; \
	  fi; \
	fi
queryparser/queryparser_internal.stamp: queryparser/queryparser.lemony queryparser/queryparser.lt queryparser/lemon
## It's OK to directly update the output file here, since it's the stamp file
## which determines whether the file is up to date.
	queryparser/lemon -q -oqueryparser/queryparser_internal.cc \
	    -hqueryparser/queryparser_token.h \
	    $(srcdir)/queryparser/queryparser.lemony
	$(PERL) -pi -e 's@^(#line \d+ ").*/(queryparser/)@$$1$$2@' \
	    queryparser/queryparser_internal.cc
## Lemon carefully avoids touching queryparser_token.h if it hasn't changed,
## but only the generated file queryparser_internal.cc depends on it, so it's
## better to touch it so we can have a dependency to generate it.
	touch queryparser/queryparser_token.h
	touch $@

BUILT_SOURCES += $(lemon_built_sources)
CLEANFILES += queryparser/lemon
endif

lib_src +=\
	queryparser/cjk-tokenizer.cc\
	queryparser/queryparser.cc\
	queryparser/queryparser_internal.cc\
	queryparser/termgenerator.cc\
	queryparser/termgenerator_internal.cc
