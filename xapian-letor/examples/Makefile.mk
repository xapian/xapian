bin_PROGRAMS +=\
	examples/questletor

examples_questletor_SOURCES = examples/questletor.cc
examples_questletor_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	examples/questletor.1
endif

if DOCUMENTATION_RULES
examples/questletor.1: examples/questletor$(EXEEXT) makemanpage
	./makemanpage examples/questletor $(srcdir)/examples/questletor.cc examples/questletor.1
endif
