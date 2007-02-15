bin_PROGRAMS +=
	examples/copydatabase\
	examples/delve\
	examples/quest\
	examples/simpleexpand\
	examples/simpleindex\
	examples/simplesearch

exaples_copydatabase_SOURCES = examples/copydatabase.cc
exaples_copydatabase_LDADD = $(ldflags) libxapian.la

exaples_delve_SOURCES = examples/delve.cc
exaples_delve_LDADD = $(ldflags) libgetopt.la libxapian.la

exaples_quest_SOURCES = examples/quest.cc
exaples_quest_LDADD = $(ldflags) libgetopt.la libxapian.la

exaples_simpleexpand_SOURCES = examples/simpleexpand.cc
exaples_simpleexpand_LDADD = $(ldflags) libxapian.la

exaples_simpleindex_SOURCES = examples/simpleindex.cc
exaples_simpleindex_LDADD = $(ldflags) libxapian.la

exaples_simplesearch_SOURCES = examples/simplesearch.cc
exaples_simplesearch_LDADD = $(ldflags) libxapian.la

dist_man_MANS +=\
	examples/copydatabase.1\
	examples/delve.1\
	examples/quest.1

if MAINTAINER_MODE
examples/copydatabase.1: examples/copydatabase
	$(HELP2MAN) -o examples/copydatabase.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/examples/copydatabase.cc`" examples/copydatabase

examples/delve.1: examples/delve
	$(HELP2MAN) -o examples/delve.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/examples/delve.cc`" examples/delve

examples/quest.1: examples/quest
	$(HELP2MAN) -o examples/quest.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/examples/quest.cc`" examples/quest
endif
