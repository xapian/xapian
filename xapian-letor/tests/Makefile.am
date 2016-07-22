## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.12.2 serial-tests subdir-objects

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/include \
 -I$(top_srcdir)/include -I$(srcdir)/harness -I../../xapian-core/include \
 -DXAPIAN_LIB_BUILD -I$(top_srcdir)/../xapian-core/include \
 -I$(top_srcdir)/../xapian-core/tests

CLEANFILES =

libxapian_la = libxapian-1.3.la

TESTS_ENVIRONMENT = ./runtest

.PHONY: check-none check-inmemory \
	check-chert check-glass \
	check-multi check-multi-chert check-multi-glass \
	check-remote check-remoteprog check-remotetcp \
	check-remoteprog-chert check-remoteprog-glass \
	check-remotetcp-chert check-remotetcp-glass \
	up remove-cached-databases

up:
	cd .. && $(MAKE)

check-none: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b none

check-inmemory: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b inmemory

check-multi: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b multi

check-singlefile: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b singlefile

if BUILD_BACKEND_REMOTE
check-remote: check-remoteprog check-remotetcp

check-remoteprog: apitest$(EXEEXT)
## If both chert and glass are enabled, just run remoteprog_chert.
if BUILD_BACKEND_CHERT
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog_chert
else
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog
endif

check-remotetcp: apitest$(EXEEXT)
## If both chert and glass are enabled, just run remotetcp_glass.
if BUILD_BACKEND_GLASS
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp_glass
else
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp
endif

if BUILD_BACKEND_GLASS
check-remoteprog-glass: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog_glass
check-remotetcp-glass: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp_glass
endif

if BUILD_BACKEND_CHERT
check-remoteprog-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remoteprog_chert
check-remotetcp-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b remotetcp_chert
endif
endif

if BUILD_BACKEND_GLASS
check-multi-glass: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b multi_glass
check-glass: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b glass
check-singlefile-glass: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b singlefile_glass
endif

if BUILD_BACKEND_CHERT
check-multi-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b multi_chert
check-chert: apitest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./apitest$(EXEEXT) -b chert
endif

## Test programs to be run
TESTS = apitest$(EXEEXT)

## Programs to build
check_PROGRAMS = \
	apitest

# Make sure runtest is up to date before running tests
check_SCRIPTS = runtest

## Sources:

noinst_HEADERS = apitest.h

collated_apitest_sources = \
 api_generated.cc

apitest_SOURCES = apitest.cc $(collated_apitest_sources) \
 api_all.h api_collated.h $(testharness_sources)

apitest_LDFLAGS = $(NO_INSTALL) $(ldflags)
apitest_LDADD = ../../xapian-core/libgetopt.la ../../xapian-core/$(libxapian_la) ../libxapianletor.la

BUILT_SOURCES =

if MAINTAINER_MODE
BUILT_SOURCES += api_all.h api_collated.h $(collated_apitest_sources:.cc=.h) \
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
api_collated.stamp: $(collated_apitest_sources) collate-test Makefile.am
	$(PERL) "$(srcdir)/collate-test" "$(srcdir)" api_collated.h api_all.h $(collated_apitest_sources)
	touch $@

api_generated.cc: generate-api_generated
	$(PERL) "$(srcdir)/generate-api_generated" > api_generated.ccT
	mv api_generated.ccT api_generated.cc
endif

EXTRA_DIST = collate-test generate-api_generated api_all.h api_collated.h \
	$(collated_apitest_sources:.cc=.h)

## Distribute test data:
EXTRA_DIST +=\
	runtest.in \
	valgrind.supp \
	testdata/apitest_rset.txt \
	testdata/apitest_phrase.txt \
	testdata/apitest_allterms4.txt

remove-cached-databases:
	rm -rf .chert .glass .multichert .multiglass .replicatmp .singlefileglass .stub

clean-local: remove-cached-databases

# Remove the cached databases before "make check" to avoid issues with missing
# bugs in database creation when we reuse databases from a previous test run.
check: remove-cached-databases

include harness/Makefile.mk

EXTRA_DIST += zlib-vg.c
if USE_ZLIB_VG
check_SCRIPTS += zlib-vg.so
CLEANFILES += zlib-vg.so

zlib-vg.so: zlib-vg.c
	$(CC_FOR_BUILD) -fPIC -shared -o zlib-vg.so $(srcdir)/zlib-vg.c -ldl
endif

.PHONY: check-syntax

check-syntax:
	$(CXXCOMPILE) -fsyntax-only -gstabs+ $(CHK_SOURCES)
