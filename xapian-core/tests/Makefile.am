## Process this file with automake to produce Makefile.in

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite

# These are for internal test - should be removed, and internal test
# split into bits among the sources.
INCLUDES += @BERKELEY_2_x_INCLUDES@

if HAVE_PERL
PERLTESTS = stemtest.pl
else
PERLTESTS =
endif

if HAVE_CPP_ACCESS_BYPASS
INTERNALTEST_CXXFLAGS = -fno-access-control -DHAVE_NO_ACCESS_CONTROL
DELVE_CXXFLAGS = -fno-access-control -DHAVE_NO_ACCESS_CONTROL
else
INTERNALTEST_CXXFLAGS =
DELVE_CXXFLAGS =
endif

if BUILD_PTHREAD_SUPPORT
THREADTESTS = threadtest
else
THREADTESTS =
endif

## Test programs to be run
TESTS = $(THREADTESTS) apitest internaltest $(PERLTESTS)

## Programs to build
check_PROGRAMS  = $(THREADTESTS) apitest internaltest stemtest delve

check-local: includetest.o

## Sources:

threadtest_SOURCES = threadtest.cc
threadtest_LDADD = ../testsuite/libtest.la ../libomus.la

apitest_SOURCES = apitest.cc
apitest_LDADD = ../testsuite/libtest.la ../libomus.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

internaltest_SOURCES = internaltest.cc
internaltest_LDADD = ../testsuite/libtest.la ../libomus.la
## Note: the next variable is not understood by automake:
## we supply our own rule in this file.
internaltest_CXXFLAGS = $(INTERNALTEST_CXXFLAGS)

delve_SOURCES = delve.cc
delve_LDADD = ../libomus.la
## Note: the next variable is not understood by automake:
## we supply our own rule in this file.
delve_CXXFLAGS = $(DELVE_CXXFLAGS)

## Distribute test data:
EXTRA_DIST = testdata/apitest_onedoc.txt \
             testdata/apitest_simpledata.txt \
             testdata/apitest_simpledata2.txt \
	     testdata/apitest_termorder.txt \
	     testdata/apitest_rset.txt \
	     testdata/apitest_phrase.txt


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

ALL_INCLUDES = `find $(top_srcdir) -name bindings -prune -o -name \*.h -not -name acconfig.h -type f -print`

## Extra rules:
includetest.cc:
	@echo "Generating includetest.cc - including all header files."
	@depends="$(ALL_INCLUDES)"; destfile=".deps-includetest-cc"; $(GENDEPS)
	@for header in $(ALL_INCLUDES) ; do echo '#include "'$$header'"' ; done > $@
	@echo '#include "$(top_srcdir)/common/omdebug.cc"' >> $@
	@echo 'int main() { return 0; }' >> $@

## Compile internaltest and delve with the extra CXXFLAGS
## We have to provide our own rules, because automake doesn't
## actually understand _CXXFLAGS, and we know of no other way
## to pass special flags to the compilation of one target.

internaltest.o: internaltest.cc
	$(CXXCOMPILE) $(internaltest_CXXFLAGS) -c ${srcdir}/internaltest.cc

internaltest.lo: internaltest.cc
	$(LTCXXCOMPILE) $(internaltest_CXXFLAGS) -c ${srcdir}/internaltest.cc

delve.o: delve.cc
	$(CXXCOMPILE) $(delve_CXXFLAGS) -c ${srcdir}/delve.cc

delve.lo: delve.cc
	$(LTCXXCOMPILE) $(delve_CXXFLAGS) -c ${srcdir}/delve.cc
