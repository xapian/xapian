## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_PROGRAMS = dbtest matchtest idxtest stemtest enquiretest

apitest_SOURCES = apitest.cc
apitest_LDADD = ../libomus.la

dbtest_SOURCES = dbtest.cc
dbtest_LDADD = ../libomus.la

matchtest_SOURCES = matchtest.cc
matchtest_LDADD = ../libomus.la

idxtest_SOURCES = idxtest.cc
idxtest_LDADD = ../libomus.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

enquiretest_SOURCES = enquiretest.cc
enquiretest_LDADD = ../libomus.la

## Test suite

check_PROGRAMS  = apitest

apitest_SOURCES = apitest.cc
apitest_LDADD   = ../libomus.la

TESTS = apitest
