### Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; also so generated files in EXTRA_DIST work
AUTOMAKE_OPTIONS = 1.5

bin_PROGRAMS = msearch delve

msearch_SOURCES = msearch.cc
msearch_LDADD = $(XAPIAN_LIBS)
msearch_CXXFLAGS = $(XAPIAN_CXXFLAGS)

delve_SOURCES = delve.cc
delve_LDADD = $(XAPIAN_LIBS)
delve_CXXFLAGS = $(XAPIAN_CXXFLAGS)

EXTRA_DIST = COPYING buildall README \
simple/README simple/Makefile simple/simpleindex.cc \
              simple/simpleexpand.cc simple/simplesearch.cc \
	      simple/simplecreate.cc
