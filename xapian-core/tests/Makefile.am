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

## Test programs to be run
TESTS = apitest internaltest $(PERLTESTS)

## Programs to build
check_PROGRAMS = apitest internaltest stemtest

if HAVE_PERL
check-local: includetest.o
endif

## Sources:

noinst_HEADERS = apitest.h api_nodb.h api_posdb.h api_db.h

apitest_SOURCES = apitest.cc api_nodb.cc api_posdb.cc api_db.cc
apitest_LDADD = ../testsuite/libtest.la ../libxapian.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../getopt/libgetopt.la ../libxapian.la

internaltest_SOURCES = internaltest.cc
internaltest_LDADD = ../testsuite/libtest.la ../libxapian.la
## Note: the next variable is not understood by automake:
## we supply our own rule in this file.
internaltest_CXXFLAGS = $(INTERNALTEST_CXXFLAGS)

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
	     testdata/apitest_manydocs.txt \
	     testdata/etext.txt \
	     findheaders.pl

CLEANFILES = includetest.cc stemtest.pl

clean-local:
	rm -f randjunk_seed_* randtext_seed_*
	rm -rf .quartz

-include .deps-includetest-cc

dist-hook:
	rm -f $(distdir)/includetest.cc

DISTCLEANFILES = .deps-includetest-cc

# Get GENDEPS macro
include $(top_srcdir)/Makefile_common.am

ALL_INCLUDES = `$(PERL) $(srcdir)/findheaders.pl $(top_srcdir)`

## Extra rules:
includetest.cc: findheaders.pl
	@echo "Generating includetest.cc - including all header files."
	depends="$(ALL_INCLUDES)"; destfile=".deps-includetest-cc"; $(GENDEPS)
	for header in $(ALL_INCLUDES) ; do echo '#include "'$$header'"' ; done > $@
	echo '#include "$(top_srcdir)/common/omdebug.cc"' >> $@
	echo 'int main() { return 0; }' >> $@

# FIXME: ought to check each header file is OK by itself ideally...
includetest.o: includetest.cc
	$(CXXCOMPILE) -c includetest.cc

## Compile internaltest with the extra CXXFLAGS
## We have to provide our own rules, because automake doesn't
## actually understand _CXXFLAGS, and we know of no other way
## to pass special flags to the compilation of one target.

internaltest.o: internaltest.cc
	$(CXXCOMPILE) $(internaltest_CXXFLAGS) -c $(srcdir)/internaltest.cc

internaltest.lo: internaltest.cc
	$(LTCXXCOMPILE) $(internaltest_CXXFLAGS) -c $(srcdir)/internaltest.cc
