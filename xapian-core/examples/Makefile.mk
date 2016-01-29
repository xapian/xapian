EXTRA_DIST +=\
	examples/Makefile

bin_PROGRAMS +=\
	examples/copydatabase\
	examples/quest\
	examples/simpleexpand\
	examples/simpleindex\
	examples/simplesearch\
	examples/xapian-metadata

examples_copydatabase_SOURCES = examples/copydatabase.cc
examples_copydatabase_LDADD = $(ldflags) $(libxapian_la)

examples_quest_SOURCES = examples/quest.cc
examples_quest_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

examples_simpleexpand_SOURCES = examples/simpleexpand.cc
examples_simpleexpand_LDADD = $(ldflags) $(libxapian_la)

examples_simpleindex_SOURCES = examples/simpleindex.cc
examples_simpleindex_LDADD = $(ldflags) $(libxapian_la)

examples_simplesearch_SOURCES = examples/simplesearch.cc
examples_simplesearch_LDADD = $(ldflags) $(libxapian_la)

examples_xapian_metadata_SOURCES = examples/xapian-metadata.cc
examples_xapian_metadata_LDADD = $(ldflags) $(libxapian_la)

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	examples/copydatabase.1\
	examples/quest.1\
	examples/xapian-metadata.1
endif

if DOCUMENTATION_RULES
examples/copydatabase.1: examples/copydatabase$(EXEEXT) makemanpage
	./makemanpage examples/copydatabase $(srcdir)/examples/copydatabase.cc examples/copydatabase.1

examples/quest.1: examples/quest$(EXEEXT) makemanpage
	./makemanpage examples/quest $(srcdir)/examples/quest.cc examples/quest.1

examples/xapian-metadata.1: examples/xapian-metadata$(EXEEXT) makemanpage
	./makemanpage examples/xapian-metadata $(srcdir)/examples/xapian-metadata.cc examples/xapian-metadata.1
endif
