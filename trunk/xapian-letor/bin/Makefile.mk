bin_PROGRAMS +=\
	bin/questletor\
	bin/xapian-letor-update

bin_questletor_SOURCES =\
	bin/questletor.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_questletor_LDADD = libxapianletor.la

bin_xapian_letor_update_SOURCES =\
	bin/xapian-letor-update.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_xapian_letor_update_LDADD = $(XAPIAN_LIBS)

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/questletor.1\
	bin/xapian-letor-update.1
endif

if DOCUMENTATION_RULES
bin/questletor.1: bin/questletor$(EXEEXT) makemanpage
	./makemanpage bin/questletor $(srcdir)/bin/questletor.cc bin/questletor.1

bin/xapian-letor-update.1: bin/xapian-letor-update$(EXEEXT) makemanpage
	./makemanpage bin/xapian-letor-update $(srcdir)/bin/xapian-letor-update.cc bin/xapian-letor-update.1
endif
