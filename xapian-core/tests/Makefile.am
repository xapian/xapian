## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

# Standard programs (these will eventually be moved / converted
# into test cases.)

noinst_PROGRAMS = matchtest stemtest

## Test suite

check_PROGRAMS  = apitest

TESTS = apitest

## Sources:

apitest_SOURCES = apitest.cc
apitest_LDADD = ../libomus.la

matchtest_SOURCES = matchtest.cc
matchtest_LDADD = ../libomus.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libomus.la

## Test data:

EXTRA_DIST = apitest_onedoc.txt apitest_simpledata.txt
