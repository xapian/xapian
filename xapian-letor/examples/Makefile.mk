bin_PROGRAMS +=\
	examples/questletor

examples_questletor_SOURCES =\
	examples/questletor.cc\
	common/getopt.cc\
	common/gnu_getopt.h
examples_questletor_LDADD = libxapianletor.la

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	examples/questletor.1
endif

if DOCUMENTATION_RULES
examples/questletor.1: examples/questletor$(EXEEXT) makemanpage
	./makemanpage examples/questletor $(srcdir)/examples/questletor.cc examples/questletor.1
endif
