bin_PROGRAMS +=\
	bin/xapian-letor-update\
	bin/xapian-rank

bin_xapian_letor_update_SOURCES =\
	bin/xapian-letor-update.cc\
	common/getopt.cc\
	common/gnu_getopt.h\
	common/str.cc
bin_xapian_letor_update_LDADD = $(XAPIAN_LIBS)

bin_xapian_rank_SOURCES =\
	bin/xapian-rank.cc\
	common/getopt.cc
bin_xapian_rank_LDADD = libxapianletor.la

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/xapian-letor-update.1\
	bin/xapian-rank.1
endif

if DOCUMENTATION_RULES
bin/xapian-letor-update.1: bin/xapian-letor-update$(EXEEXT) makemanpage
	./makemanpage bin/xapian-letor-update $(srcdir)/bin/xapian-letor-update.cc bin/xapian-letor-update.1

bin/xapian-rank.1: bin/xapian-rank$(EXEEXT) makemanpage
	./makemanpage bin/xapian-rank $(srcdir)/bin/xapian-rank.cc bin/xapian-rank.1
endif
