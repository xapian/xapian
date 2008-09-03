## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 subdir-objects

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
 -I$(top_builddir)/include -I$(srcdir)/harness

TESTS_ENVIRONMENT = ./runtest

.PHONY: check-none check-inmemory \
        check-multi check-multi-chert check-multi-flint \
        check-remote \
	check-remoteprog check-remoteprog-chert check-remoteprog-flint\
	check-remotetcp check-remotetcp-chert check-remotetcp-flint\
        check-chert check-flint up remove-cached-databases

up:
	cd .. && $(MAKE)

check-none: apitest$(EXEEXT) \
            internaltest$(EXEEXT) \
	    stemtest$(EXEEXT) \
	    queryparsertest$(EXEEXT) \
	    termgentest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b none
	$(TESTS_ENVIRONMENT) ./internaltest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./stemtest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./queryparsertest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./termgentest$(EXEEXT)

check-inmemory: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b inmemory

check-multi: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b multi

if BUILD_BACKEND_REMOTE
check-remote: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp
check-remoteprog: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog
check-remotetcp: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp
if BUILD_BACKEND_CHERT
check-remoteprog-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog_chert
check-remotetcp-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp_chert
endif
if BUILD_BACKEND_FLINT
check-remoteprog-flint: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog_flint
check-remotetcp-flint: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp_flint
endif
endif

if BUILD_BACKEND_CHERT
check-multi-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b multi_chert
check-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b chert
endif

if BUILD_BACKEND_FLINT
check-multi-flint: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b multi_flint
check-flint: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b flint
endif

## Test programs to be run
TESTS = apitest$(EXEEXT) internaltest$(EXEEXT) stemtest$(EXEEXT) \
 queryparsertest$(EXEEXT) termgentest$(EXEEXT)

## Programs to build
check_PROGRAMS = apitest internaltest stemtest queryparsertest termgentest

# Make sure runtest is up to date before running tests
check_SCRIPTS = runtest

## Sources:

noinst_HEADERS = apitest.h

collated_apitest_sources = \
 api_anydb.cc \
 api_db.cc \
 api_generated.cc \
 api_nodb.cc \
 api_percentages.cc \
 api_posdb.cc \
 api_replicate.cc \
 api_sorting.cc \
 api_transdb.cc \
 api_unicode.cc \
 api_valuestats.cc \
 api_wrdb.cc

apitest_SOURCES = apitest.cc $(collated_apitest_sources) \
 api_all.h api_collated.h $(testharness_sources)

apitest_LDFLAGS = -no-install $(ldflags)
apitest_LDADD = ../libgetopt.la ../libxapian.la

stemtest_SOURCES = stemtest.cc $(testharness_sources)
stemtest_LDFLAGS = -no-install $(ldflags)
stemtest_LDADD = ../libgetopt.la ../libxapian.la

internaltest_SOURCES = internaltest.cc $(testharness_sources)
internaltest_LDFLAGS = -no-install $(ldflags)
internaltest_LDADD = ../libgetopt.la ../libxapian.la

queryparsertest_SOURCES = queryparsertest.cc $(testharness_sources)
queryparsertest_LDFLAGS = -no-install $(ldflags)
queryparsertest_LDADD = ../libgetopt.la ../libxapian.la

termgentest_SOURCES = termgentest.cc $(testharness_sources)
termgentest_LDFLAGS = -no-install $(ldflags)
termgentest_LDADD = ../libgetopt.la ../libxapian.la

BUILT_SOURCES =

if MAINTAINER_MODE
BUILT_SOURCES += api_all.h api_collated.h $(collated_apitest_sources:.cc=.h) \
 api_generated.cc
CLEANFILES = api_all.h api_collated.h $(collated_apitest_sources:.cc=.h) \
 api_generated.cc

api_all.h api_collated.h $(collated_apitest_sources:.cc=.h): api_collated.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf api_collated.lock api_collated.stamp' 1 2 13 15; \
	  if mkdir api_collated.lock 2>/dev/null; then \
	    rm -f api_collated.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) api_collated.stamp; \
	    rmdir api_collated.lock; \
	  else \
	    while test -d api_collated.lock; do sleep 1; done; \
	    test -f api_collated.stamp; exit $$?; \
	  fi; \
	fi
api_collated.stamp: $(collated_apitest_sources) collate-test
	$(PERL) "$(srcdir)/collate-test" "$(srcdir)" api_all.h $(collated_apitest_sources) > api_collated.hT
	mv api_collated.hT api_collated.h
	touch $@

api_generated.cc: generate-api_generated
	$(PERL) "$(srcdir)/generate-api_generated" > api_generated.ccT
	mv api_generated.ccT api_generated.cc
endif

EXTRA_DIST = collate-test generate-api_generated api_all.h api_collated.h \
	$(collated_apitest_sources:.cc=.h) dir_contents

## Distribute test data:
EXTRA_DIST +=\
	runtest.in \
	valgrind.supp \
	testdata/apitest_onedoc.txt \
	testdata/apitest_simpledata.txt \
	testdata/apitest_simpledata2.txt \
	testdata/apitest_termorder.txt \
	testdata/apitest_rset.txt \
	testdata/apitest_phrase.txt \
	testdata/apitest_punc.txt \
	testdata/apitest_space.txt \
	testdata/apitest_allterms.txt \
	testdata/apitest_allterms2.txt \
	testdata/apitest_allterms4.txt \
	testdata/apitest_poslist.txt \
	testdata/apitest_manydocs.txt \
	testdata/apitest_sortconsist.txt \
	testdata/apitest_sortrel.txt \
	testdata/etext.txt \
	testdata/flint-0.9.9/flicklock \
	testdata/flint-0.9.9/iamflint \
	testdata/flint-0.9.9/position.baseA \
	testdata/flint-0.9.9/position.DB \
	testdata/flint-0.9.9/postlist.baseA\
	testdata/flint-0.9.9/postlist.DB \
	testdata/flint-0.9.9/record.baseA \
	testdata/flint-0.9.9/record.DB \
	testdata/flint-0.9.9/termlist.baseA \
	testdata/flint-0.9.9/termlist.DB \
	testdata/flint-0.9.9/value.baseA \
	testdata/flint-0.9.9/value.DB \
	testdata/flint-1.0.1/flintlock \
	testdata/flint-1.0.1/iamflint \
	testdata/flint-1.0.1/position.baseA \
	testdata/flint-1.0.1/position.DB \
	testdata/flint-1.0.1/postlist.baseA \
	testdata/flint-1.0.1/postlist.DB \
	testdata/flint-1.0.1/record.baseA \
	testdata/flint-1.0.1/record.DB \
	testdata/flint-1.0.1/termlist.baseA \
	testdata/flint-1.0.1/termlist.DB \
	testdata/flint-1.0.1/value.baseA \
	testdata/flint-1.0.1/value.DB \
	testdata/flint-1.0.2/flintlock \
	testdata/flint-1.0.2/iamflint \
	testdata/flint-1.0.2/postlist.baseA \
	testdata/flint-1.0.2/postlist.DB \
	testdata/flint-1.0.2/record.baseA \
	testdata/flint-1.0.2/record.DB \
	testdata/flint-1.0.2/termlist.baseA \
	testdata/flint-1.0.2/termlist.DB

remove-cached-databases:
	rm -rf .chert .flint .multi .multichert .multiflint .stub

clean-local: remove-cached-databases

# Remove the cached databases before "make check" to avoid issues with missing
# bugs in database creation when we reuse databases from a previous test run.
check-local: remove-cached-databases

include perftest/Makefile.mk
include harness/Makefile.mk
