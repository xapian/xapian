bin_PROGRAMS +=\
	bin/xapian-letor-update\
	bin/xapian-prepare-trainingfile\
	bin/xapian-rank\
	bin/xapian-train

bin_xapian_letor_update_SOURCES =\
	bin/xapian-letor-update.cc
bin_xapian_letor_update_LDADD = libcommon_str.la libgetopt.la $(XAPIAN_LIBS)

bin_xapian_prepare_trainingfile_SOURCES =\
	bin/xapian-prepare-trainingfile.cc
bin_xapian_prepare_trainingfile_LDADD = libgetopt.la libxapianletor.la $(XAPIAN_LIBS)

bin_xapian_rank_SOURCES =\
	bin/xapian-rank.cc
bin_xapian_rank_LDADD = libgetopt.la libxapianletor.la $(XAPIAN_LIBS)

bin_xapian_train_SOURCES =\
	bin/xapian-train.cc
bin_xapian_train_LDADD = libgetopt.la libxapianletor.la $(XAPIAN_LIBS)

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/xapian-letor-update.1\
	bin/xapian-prepare-trainingfile.1\
	bin/xapian-rank.1\
	bin/xapian-train.1
endif

if DOCUMENTATION_RULES
bin/xapian-letor-update.1: bin/xapian-letor-update$(EXEEXT) makemanpage
	./makemanpage bin/xapian-letor-update $(srcdir)/bin/xapian-letor-update.cc bin/xapian-letor-update.1

bin/xapian-prepare-trainingfile.1: bin/xapian-prepare-trainingfile$(EXEEXT) makemanpage
	./makemanpage bin/xapian-prepare-trainingfile $(srcdir)/bin/xapian-prepare-trainingfile.cc bin/xapian-prepare-trainingfile.1

bin/xapian-rank.1: bin/xapian-rank$(EXEEXT) makemanpage
	./makemanpage bin/xapian-rank $(srcdir)/bin/xapian-rank.cc bin/xapian-rank.1

bin/xapian-train.1: bin/xapian-train$(EXEEXT) makemanpage
	./makemanpage bin/xapian-train $(srcdir)/bin/xapian-train.cc bin/xapian-train.1
endif
