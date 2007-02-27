## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 subdir-objects

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
 -I$(top_builddir)/include -I$(srcdir)/harness

TESTS_ENVIRONMENT = ./runtest

.PHONY: check-remote check-flint check-quartz

if BUILD_BACKEND_REMOTE
check-remote: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remote
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp
endif

if BUILD_BACKEND_FLINT
check-flint: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b flint
endif

if BUILD_BACKEND_QUARTZ
check-quartz: btreetest$(EXEEXT) quartztest$(EXEEXT) apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./btreetest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./quartztest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b quartz

quartz_check_programs = btreetest quartztest
quartz_tests= btreetest$(EXEEXT) quartztest$(EXEEXT)

# Only quartztest and btreetest need this, but if we put it in INCLUDES
# then we can use the same objects for the testsuite harness for all
# test programs.
INCLUDES += -I$(top_srcdir)/backends/quartz
endif

## Test programs to be run
TESTS = $(quartz_tests) \
 apitest$(EXEEXT) internaltest$(EXEEXT) stemtest$(EXEEXT) \
 queryparsertest$(EXEEXT)

## Programs to build
check_PROGRAMS = $(quartz_check_programs) \
 apitest internaltest stemtest queryparsertest

# Make sure runtest is up to date before running tests
check_SCRIPTS = runtest

## Sources:

noinst_HEADERS = apitest.h api_nodb.h api_posdb.h api_db.h api_wrdb.h \
 api_anydb.h api_transdb.h

apitest_SOURCES = apitest.cc api_nodb.cc api_posdb.cc api_db.cc api_wrdb.cc \
 api_anydb.cc api_transdb.cc $(testharness_sources)
apitest_LDFLAGS = @ldflags@
apitest_LDADD = ../libgetopt.la ../libxapian.la

stemtest_SOURCES = stemtest.cc $(testharness_sources)
stemtest_LDFLAGS = @ldflags@
stemtest_LDADD = ../libgetopt.la ../libxapian.la

internaltest_SOURCES = internaltest.cc $(testharness_sources)
internaltest_LDFLAGS = @ldflags@
internaltest_LDADD = ../libgetopt.la ../libxapian.la

btreetest_SOURCES = btreetest.cc $(testharness_sources)
btreetest_LDFLAGS = @ldflags@
btreetest_LDADD = ../libgetopt.la ../libquartzcheck.la ../libxapian.la

quartztest_SOURCES = quartztest.cc $(testharness_sources)
quartztest_LDFLAGS = @ldflags@
quartztest_LDADD = ../libgetopt.la ../libxapian.la

queryparsertest_SOURCES = queryparsertest.cc $(testharness_sources)
queryparsertest_LDFLAGS = @ldflags@
queryparsertest_LDADD = ../libgetopt.la ../libxapian.la

## Distribute test data:
EXTRA_DIST = runtest.in \
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
	     testdata/apitest_sortrel.txt \
	     testdata/etext.txt \
	     testdata/btreetest_ord+ \
	     testdata/btreetest_ord- \
	     testdata/btreetest_ordnum+ \
	     testdata/btreetest_ordnum-

clean-local:
	rm -rf .flint .quartz .quartztmp .btreetmp

include harness/Makefile.mk
