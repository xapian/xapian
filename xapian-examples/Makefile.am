### Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CXXFLAGS, etc; 1.6.3 for AM_INIT_AUTOMAKE with no args
# 1.8.5 contains a lot of fixes over 1.6.3
AUTOMAKE_OPTIONS = 1.8.5

bin_PROGRAMS = quest delve copydatabase\
	simpleindex simplesearch simpleexpand

AM_CXXFLAGS = @AM_CXXFLAGS@

noinst_HEADERS = gnu_getopt.h

quest_SOURCES = quest.cc getopt.cc
quest_LDADD = $(XAPIAN_LIBS)

delve_SOURCES = delve.cc getopt.cc
delve_LDADD = $(XAPIAN_LIBS)

copydatabase_SOURCES = copydatabase.cc
copydatabase_LDADD = $(XAPIAN_LIBS)

simpleindex_SOURCES = simpleindex.cc
simpleindex_LDADD = $(XAPIAN_LIBS)

simplesearch_SOURCES = simplesearch.cc
simplesearch_LDADD = $(XAPIAN_LIBS)

simpleexpand_SOURCES = simpleexpand.cc
simpleexpand_LDADD = $(XAPIAN_LIBS)
