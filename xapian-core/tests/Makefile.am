## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

if HAVE_PERL
PERLTESTS = stemtest.pl
else
PERLTESTS = 
endif

## Test programs to be run
TESTS = apitest includetest $(PERLTESTS)

## Programs to build
check_PROGRAMS  = apitest includetest stemtest

## Sources:

includetest_SOURCES = includetest.cc

apitest_SOURCES = apitest.cc
apitest_LDADD = ../libomus.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

## Test data:

EXTRA_DIST = testdata/apitest_onedoc.txt \
             testdata/apitest_simpledata.txt \
             testdata/apitest_simpledata2.txt \
	     testdata/apitest_termorder.txt


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
