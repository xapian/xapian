## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite

# These are for internal test - should be removed, and internal test
# split into bits among the sources.
INCLUDES += -I/usr/local/BerkeleyDB/include -I/usr/include/db2

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

## Test programs to be run
TESTS = apitest internaltest includetest $(PERLTESTS)

## Programs to build
check_PROGRAMS  = apitest internaltest includetest stemtest

## Sources:

includetest_SOURCES = includetest.cc

apitest_SOURCES = apitest.cc
apitest_LDADD = ../testsuite/libtest.la ../libomus.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

EXTRA_internaltest_SOURCES = internaltest.cc
internaltest_SOURCES = internaltest_dummy.cc
internaltest_LDADD = ../libomus.la ../testsuite/libtest.la
## Note: the next variable is not understood by automake:
## we supply our own rule in this file.
internaltest_CXXFLAGS = $(INTERNALTEST_CXXFLAGS)

## Distribute test data:
EXTRA_DIST = testdata/apitest_onedoc.txt \
             testdata/apitest_simpledata.txt \
             testdata/apitest_simpledata2.txt \
	     testdata/apitest_termorder.txt \
	     testdata/apitest_rset.txt


clean-local:
	@if [ -e includetest.cc ] ; then \
	  echo "removing includetest.cc"; $(RM) includetest.cc ; \
	fi
	@if [ -e stemtest.pl ] ; then \
	  echo "removing stemtest.pl"; $(RM) stemtest.pl ; \
	fi
	@echo "removing random stemtest data"; \
	$(RM) randjunk_seed_* ; \
	$(RM) randtext_seed_* ;

-include .deps-includetest-cc

dist-hook:
	@if [ -e $(distdir)/includetest.cc ] ; then \
	  echo "removing includetest.cc from distribution"; \
	  $(RM) $(distdir)/includetest.cc ; \
	fi
	@if [ -e $(distdir)/internaltest_dummy.cc ] ; then \
	  echo "removing internaltest_dummy.cc from distribution"; \
	  $(RM) $(distdir)/internaltest_dummy.cc ; \
	fi

distclean-local: clean-local
	@if [ -e .deps-includetest-cc ] ; then \
	  echo "removing .deps-includetest-cc"; $(RM) .deps-includetest-cc ; \
	fi

# Get GENDEPS macro
include $(top_srcdir)/Makefile_common.am

ALL_INCLUDES = `find $(top_srcdir) \( -name \*.h -and -not -name acconfig.h \) -print`

## Extra rules:
includetest.cc: 
	@echo "Generating includetest.cc - including all header files."
	@depends="$(ALL_INCLUDES)"; destfile=".deps-includetest-cc"; $(GENDEPS)
	@for header in $(ALL_INCLUDES) ; do echo $$header ; done | \
	grep -v sleepy | sed 's/\(.*\)/#include "\1"/' > $@
	@echo 'int main() { return 0; }' >> $@

## Compile internaltest with the extra CXXFLAGS
## We have to provide our own rules, because automake doesn't
## actually understand _CXXFLAGS, and we know of no other way
## to pass special flags to the compilation of one target.

# Make an empty file: this is so "make dist" doesn't choke
# We remove this from the actual distribution in dist-hook
internaltest_dummy.cc:
	@touch $@

# Build from the file without the _dummy, with the extra flags
internaltest_dummy.o:
	@$(MAKE) CXXFLAGS="$(CXXFLAGS) $(internaltest_CXXFLAGS)" internaltest.o
	@mv internaltest.o $@
internaltest_dummy.lo: internaltest.cc
	@$(MAKE) CXXFLAGS="$(CXXFLAGS) $(internaltest_CXXFLAGS)" internaltest.lo
	@mv internaltest.lo $@
