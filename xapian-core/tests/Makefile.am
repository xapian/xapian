## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

# These are for internal test - should be removed, and internal test
# split into bits among the sources.
INCLUDES += -I/usr/local/BerkeleyDB/include -I/usr/include/db2

if HAVE_PERL
PERLTESTS = stemtest.pl
else
PERLTESTS = 
endif

if HAVE_CPP_ACCESS_BYPASS
INTERNALTEST_CFLAGS = -fno-access-control -DHAVE_NO_ACCESS_CONTROL
else
INTERNALTEST_CFLAGS =
endif

## Test programs to be run
TESTS = apitest internaltest includetest $(PERLTESTS)

## Programs to build
check_PROGRAMS  = apitest internaltest includetest stemtest

## Sources:

includetest_SOURCES = includetest.cc

apitest_SOURCES = apitest.cc
apitest_LDADD = ../libomus.la ../testsuite/libtest.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

internaltest_SOURCES = internaltest.cc
internaltest_LDADD = ../libomus.la ../testsuite/libtest.la
## Note: the next variable is not understood by automake:
## we supply our own rule in this file.
internaltest_CFLAGS = $(INTERNALTEST_CFLAGS)

## Distribute test data:
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

dist-hook:
	@if [ -e $(distdir)/includetest.cc ] ; then \
	  echo "removing includetest.cc from distribution"; \
	  $(RM) $(distdir)/includetest.cc ; \
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

## Compile internaltest with the extra CFLAGS
## We have to provide our own rule, because automake doesn't
## actually understand CFLAGS, and we know of no other way
## to pass special flags to the compilation of one target.
internaltest.o: internaltest.cc
	@echo 'Compiling $<'; \
	$(CXXCOMPILE) -Wp,-MD,.deps/$(*F).pp $(internaltest_CFLAGS) -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp


internaltest.lo: internaltest.cc
	@echo 'Compiling $<'; \
	$(LTCXXCOMPILE) -Wp,-MD,.deps/$(*F).pp $(internaltest_CFLAGS) -c $<
	@-sed -e 's/^\([^:]*\)\.o[      ]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp
