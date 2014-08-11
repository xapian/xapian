bin_PROGRAMS +=\
	bin/letor-request\
	bin/letor-prepare\
	bin/letor-train\
	bin/letor-select

bin_letor_request_SOURCES =\
	bin/letor-request.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_letor_request_LDADD = libxapianletor.la

bin_letor_prepare_SOURCES =\
	bin/letor-prepare.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_letor_prepare_LDADD = libxapianletor.la

bin_letor_train_SOURCES =\
	bin/letor-train.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_letor_train_LDADD = libxapianletor.la

bin_letor_select_SOURCES =\
	bin/letor-select.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_letor_select_LDADD = libxapianletor.la

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/letor-request.1\
	bin/letor-prepare.1\
	bin/letor-train.1\
	bin/letor-select.1
endif

if DOCUMENTATION_RULES
bin/letor-request.1: bin/letor-request$(EXEEXT) makemanpage
	./makemanpage bin/letor-request$(srcdir)/bin/letor-request.cc bin/letor-request.1

bin/letor-prepare.1: bin/letor-prepare$(EXEEXT) makemanpage
	./makemanpage bin/letor-prepare$(srcdir)/bin/letor-prepare.cc bin/letor-prepare.1

bin/letor-train.1: bin/letor-train$(EXEEXT) makemanpage
	./makemanpage bin/letor-train$(srcdir)/bin/letor-train.cc bin/letor-train.1

bin/letor-select.1: bin/letor-select$(EXEEXT) makemanpage
	./makemanpage bin/letor-select$(srcdir)/bin/letor-select.cc bin/letor-select.1
endif
