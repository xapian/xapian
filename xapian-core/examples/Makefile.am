### Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

bin_PROGRAMS = quest delve copydatabase\
	simpleindex simplesearch simpleexpand

quest_SOURCES = quest.cc
quest_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

delve_SOURCES = delve.cc
delve_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

copydatabase_SOURCES = copydatabase.cc
copydatabase_LDADD = @ldflags@ ../libxapian.la

simpleindex_SOURCES = simpleindex.cc
simpleindex_LDADD = @ldflags@ ../libxapian.la

simplesearch_SOURCES = simplesearch.cc
simplesearch_LDADD = @ldflags@ ../libxapian.la

simpleexpand_SOURCES = simpleexpand.cc
simpleexpand_LDADD = @ldflags@ ../libxapian.la
