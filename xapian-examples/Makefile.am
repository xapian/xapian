### Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; 1.6.3 for AM_INIT_AUTOMAKE with no args
AUTOMAKE_OPTIONS = 1.6.3

bin_PROGRAMS = quest delve copydatabase\
	simpleindex simplesearch simpleexpand

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS)

noinst_HEADERS = getopt.h

quest_SOURCES = quest.cc getopt.c getopt1.c
quest_LDADD = $(XAPIAN_LIBS)

# delve doesn't use getopt_long so doesn't need getopt1.c
delve_SOURCES = delve.cc getopt.c
delve_LDADD = $(XAPIAN_LIBS)

copydatabase_SOURCES = copydatabase.cc
copydatabase_LDADD = $(XAPIAN_LIBS)

simpleindex_SOURCES = simpleindex.cc
simpleindex_LDADD = $(XAPIAN_LIBS)

simplesearch_SOURCES = simplesearch.cc
simplesearch_LDADD = $(XAPIAN_LIBS)

simpleexpand_SOURCES = simpleexpand.cc
simpleexpand_LDADD = $(XAPIAN_LIBS)
