## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

# Standard programs (these will eventually be moved / converted
# into test cases.)

noinst_PROGRAMS = stemtest

## Test suite

check_PROGRAMS  = apitest includetest

if HAVE_PERL
PERLTESTS = stemtest.pl
else
PERLTESTS = 
endif

TESTS = apitest includetest $(PERLTESTS)

## Sources:

includetest_SOURCES = includetest.cc

apitest_SOURCES = apitest.cc
apitest_LDADD = ../libomus.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

## Test data:

EXTRA_DIST = testdata/apitest_onedoc.txt \
             testdata/apitest_simpledata.txt \
             testdata/apitest_simpledata2.txt

# Cunning piece of code to generate a dependency file (filename in shell
# variable destfile), for target $@, mostly taken from docs/Makefile.am
GENDEPS = \
echo "$@ : " $$depends > $$destfile ; \
for p in $$depends ; do \
  echo $$p | sed -e 's/^\\$$//' -e '/^$$/d' -e '/:$$/d' | \
  sed -e 's/$$/ :/' >> $$destfile ; \
done ;

ALL_INCLUDES = `find $(top_srcdir) -name \*.h -print`

-include .deps-includetest-cc

## Extra rules:
includetest.cc: 
	@depends="$(ALL_INCLUDES)"; destfile=".deps-includetest-cc"; $(GENDEPS)
	for header in $(ALL_INCLUDES) ; do \
	   echo $$header ;\
	done | grep -v sleepy | sed 's/\(.*\)/#include "\1"/' > $@
	echo 'int main() { return 0; }' >> $@
