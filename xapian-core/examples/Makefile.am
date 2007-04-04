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

dist_man_MANS = quest.1 delve.1 copydatabase.1

MAINTAINERCLEANFILES = $(dist_man_MANS)

if MAINTAINER_MODE
CLEANFILES = $(dist_man_MANS)

quest.1: quest
	$(HELP2MAN) -o quest.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/quest.cc`" ./quest

delve.1: delve
	$(HELP2MAN) -o delve.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/delve.cc`" ./delve

copydatabase.1: copydatabase
	$(HELP2MAN) -o copydatabase.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/copydatabase.cc`" ./copydatabase
endif
