EXTRA_DIST +=\
	examples/dir_contents\
	examples/Makefile

bin_PROGRAMS +=\
	examples/copydatabase\
	examples/delve\
	examples/quest\
	examples/simpleexpand\
	examples/simpleindex\
	examples/simplesearch

# Automake (up to version 1.10, at least) has a bug causing it to miss the
# generated files in .libs/ due to bin_PROGRAMS from the clean target.
# We work around this with a clean-local: rule, in the top level Makefile.am
extra_cleandirs += examples/.libs examples/_libs

examples_copydatabase_SOURCES = examples/copydatabase.cc
examples_copydatabase_LDADD = $(ldflags) libxapian.la

examples_delve_SOURCES = examples/delve.cc
examples_delve_LDADD = $(ldflags) libgetopt.la libxapian.la

examples_quest_SOURCES = examples/quest.cc
examples_quest_LDADD = $(ldflags) libgetopt.la libxapian.la

examples_simpleexpand_SOURCES = examples/simpleexpand.cc
examples_simpleexpand_LDADD = $(ldflags) libxapian.la

examples_simpleindex_SOURCES = examples/simpleindex.cc
examples_simpleindex_LDADD = $(ldflags) libxapian.la

examples_simplesearch_SOURCES = examples/simplesearch.cc
examples_simplesearch_LDADD = $(ldflags) libxapian.la

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	examples/copydatabase.1\
	examples/delve.1\
	examples/quest.1
endif

if DOCUMENTATION_RULES
examples/copydatabase.1: examples/copydatabase$(EXEEXT) makemanpage
	./makemanpage examples/copydatabase $(srcdir)/examples/copydatabase.cc examples/copydatabase.1

examples/delve.1: examples/delve$(EXEEXT) makemanpage
	./makemanpage examples/delve $(srcdir)/examples/delve.cc examples/delve.1

examples/quest.1: examples/quest$(EXEEXT) makemanpage
	./makemanpage examples/quest $(srcdir)/examples/quest.cc examples/quest.1
endif
