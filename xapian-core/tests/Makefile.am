## Process this file with automake to produce Makefile.in

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite

if HAVE_PERL
PERLTESTS = stemtest.pl
else
PERLTESTS =
endif

if HAVE_CPP_ACCESS_BYPASS
INTERNALTEST_CXXFLAGS = -fno-access-control -DHAVE_NO_ACCESS_CONTROL
else
INTERNALTEST_CXXFLAGS =
endif

if BUILD_PTHREAD_SUPPORT
THREADTESTS = threadtest
else
THREADTESTS =
endif

if BUILD_INDEXER
INDEXERTESTS = indextest
INDEXERTESTS_RUN = run-indextest
else
INDEXERTESTS =
INDEXERTESTS_RUN =
endif

## Test programs to be run
TESTS = $(THREADTESTS) \
        run-apitest \
        run-internaltest \
        $(INDEXERTESTS_RUN) \
	$(PERLTESTS)

## Programs to build
check_PROGRAMS = $(THREADTESTS) \
                 apitest \
		 internaltest \
		 $(INDEXERTESTS) \
		 stemtest \
		 delve

if HAVE_PERL
check-local: includetest.o
endif

## Sources:

noinst_HEADERS = apitest.h api_nodb.h api_posdb.h api_db.h

threadtest_SOURCES = threadtest.cc
threadtest_LDADD = ../testsuite/libtest.la ../libxapian.la

apitest_SOURCES = apitest.cc api_nodb.cc api_posdb.cc api_db.cc
apitest_LDADD = ../testsuite/libtest.la ../libxapian.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libxapian.la

internaltest_SOURCES = internaltest.cc
internaltest_LDADD = ../testsuite/libtest.la ../libxapian.la
## Note: the next variable is not understood by automake:
## we supply our own rule in this file.
internaltest_CXXFLAGS = $(INTERNALTEST_CXXFLAGS)

indextest_SOURCES = indextest.cc
indextest_LDADD = ../testsuite/libtest.la ../libxapian.la

delve_SOURCES = delve.cc
delve_LDADD = ../libxapian.la

## Distribute test data:
EXTRA_DIST = testdata/apitest_onedoc.txt \
             testdata/apitest_simpledata.txt \
             testdata/apitest_simpledata2.txt \
	     testdata/apitest_termorder.txt \
	     testdata/apitest_rset.txt \
	     testdata/apitest_phrase.txt \
	     testdata/apitest_punc.txt \
	     testdata/apitest_space.txt \
	     testdata/apitest_allterms.txt \
	     testdata/apitest_allterms2.txt \
	     testdata/apitest_poslist.txt \
	     testdata/indextest_filereader.txt \
	     testdata/apitest_manydocs.txt


clean-local:
	@if [ -f includetest.cc ] ; then \
	  echo "removing includetest.cc"; $(RM) includetest.cc ; \
	fi
	@if [ -f stemtest.pl ] ; then \
	  echo "removing stemtest.pl"; $(RM) stemtest.pl ; \
	fi
	@echo "removing random stemtest data"; \
	$(RM) randjunk_seed_* ; \
	$(RM) randtext_seed_* ;
	@if [ -d .quartz ]; then \
	  echo "removing .quartz/"; \
	  $(RM) -r .quartz/* ; rmdir .quartz ; \
        fi

-include .deps-includetest-cc

dist-hook:
	@if [ -f $(distdir)/includetest.cc ] ; then \
	  echo "removing includetest.cc from distribution"; \
	  $(RM) $(distdir)/includetest.cc ; \
	fi

distclean-local: clean-local
	@if [ -f .deps-includetest-cc ] ; then \
	  echo "removing .deps-includetest-cc"; $(RM) .deps-includetest-cc ; \
	fi

# Get GENDEPS macro
include $(top_srcdir)/Makefile_common.am

ALL_INCLUDES = `./findheaders.pl $(top_srcdir)`

## Extra rules:
includetest.cc: findheaders.pl
	@echo "Generating includetest.cc - including all header files."
	@depends="$(ALL_INCLUDES)"; destfile=".deps-includetest-cc"; $(GENDEPS)
	@for header in $(ALL_INCLUDES) ; do echo '#include "'$$header'"' ; done > $@
	@echo '#include "$(top_srcdir)/common/omdebug.cc"' >> $@
	@echo 'int main() { return 0; }' >> $@

includetest_CXXFLAGS = @LIBXML_CFLAGS@

includetest.o: includetest.cc
	$(CXXCOMPILE) $(includetest_CXXFLAGS) -c includetest.cc

## Compile internaltest with the extra CXXFLAGS
## We have to provide our own rules, because automake doesn't
## actually understand _CXXFLAGS, and we know of no other way
## to pass special flags to the compilation of one target.

internaltest.o: internaltest.cc
	$(CXXCOMPILE) $(internaltest_CXXFLAGS) -c ${srcdir}/internaltest.cc

internaltest.lo: internaltest.cc
	$(LTCXXCOMPILE) $(internaltest_CXXFLAGS) -c ${srcdir}/internaltest.cc

## Make links to a wrapper script which adds the LD_PRELOAD for malloc
## checking
run-apitest: apitest
	@rm -f run-apitest
	$(LN_S) $(top_srcdir)/testsuite/malloc_wrapper.sh run-apitest

run-internaltest: internaltest
	@rm -f run-internaltest
	$(LN_S) $(top_srcdir)/testsuite/malloc_wrapper.sh run-internaltest

run-indextest: indextest
	@rm -f run-indextest
	$(LN_S) $(top_srcdir)/testsuite/malloc_wrapper.sh run-indextest
