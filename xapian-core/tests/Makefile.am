## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# The including of /common should be removed ultimately.
# ie: when we have an API
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

bin_PROGRAMS = dbtest matchtest idxtest stemtest

dbtest_SOURCES = dbtest.cc
dbtest_LDADD = ../libom.la

matchtest_SOURCES = matchtest.cc
matchtest_LDADD = ../libom.la

idxtest_SOURCES = idxtest.cc
idxtest_LDADD = ../libom.la

stemtest_SOURCES = stemtest.cc
stemtest_LDADD = ../libom.la
