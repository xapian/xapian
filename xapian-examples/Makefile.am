### Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; also so generated files in EXTRA_DIST work
AUTOMAKE_OPTIONS = 1.5

bin_PROGRAMS = msearch delve createdatabase simpleindex

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS)

noinst_HEADERS = getopt.h

msearch_SOURCES = msearch.cc getopt.c getopt1.c
msearch_LDADD = $(XAPIAN_LIBS)

# delve doesn't use getopt_long so doesn't need getopt1.c
delve_SOURCES = delve.cc getopt.c
delve_LDADD = $(XAPIAN_LIBS)

createdatabase_SOURCES = createdatabase.cc
createdatabase_LDADD = $(XAPIAN_LIBS)

simpleindex_SOURCES = simpleindex.cc
simpleindex_LDADD = $(XAPIAN_LIBS)

EXTRA_DIST = COPYING buildall README \
simple/README simple/Makefile simple/simpleindex.cc \
              simple/simpleexpand.cc simple/simplesearch.cc
