## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite -I${top_builddir}/include

## Test programs to be run
TESTS = apitest internaltest stemtest

## Programs to build
check_PROGRAMS = apitest internaltest stemtest

## Sources:

noinst_HEADERS = apitest.h api_nodb.h api_posdb.h api_db.h

apitest_SOURCES = apitest.cc api_nodb.cc api_posdb.cc api_db.cc
apitest_LDADD = ../testsuite/libtest.la ../libxapian.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../testsuite/libtest.la ../libxapian.la

internaltest_SOURCES = internaltest.cc
internaltest_LDADD = ../testsuite/libtest.la ../libxapian.la
if HAVE_CPP_ACCESS_BYPASS
internaltest_CXXFLAGS = -fno-access-control -DHAVE_NO_ACCESS_CONTROL
else
internaltest_CXXFLAGS =
endif

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

clean-local:
	rm -rf .quartz
