### Process this file with automake to produce Makefile.in

INCLUDES = $(XAPIAN_CFLAGS)

bin_PROGRAMS = msearch delve

msearch_SOURCES = msearch.cc
msearch_LDADD = @XAPIAN_LIBS@

delve_SOURCES = delve.cc
delve_LDADD = @XAPIAN_LIBS@

EXTRA_DIST = COPYING buildall README \
simple/README simple/Makefile simple/simpleindex.cc \
              simple/simpleexpand.cc simple/simplesearch.cc \
	      simple/simplecreate.cc
