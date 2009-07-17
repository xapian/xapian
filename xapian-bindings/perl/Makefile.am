## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = prove -I.libs

## Test programs to be run
TESTS = t/01use.t t/02pod.t t/03podcoverage.t t/04functions.t \
	t/create.t t/databasemodified.t t/document.t t/exception.t \
	t/index.t t/parser.t t/search.t t/sorter.t t/termgenerator.t \
	t/thread.t t/tied.t t/valuerange.t t/writabledatabase.t

BUILT_SOURCES = xapian_wrap.cc

EXTRA_DIST = util.i except.i $(TESTS) $(BUILT_SOURCES)

SUBDIRS = docs

perllibdir = $(PERL_LIB)/auto/Search/Xapian
perllib_LTLIBRARIES = xapian.la

perlmoddir = $(PERL_LIB)/Search
perlmod_DATA = Xapian.pm

perlPODdir = $(PERL_LIB)/Search/Xapian
perlPOD_DATA = lib/*

AM_CPPFLAGS = -I$(PERL_INC)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module -shrext $(PERL_SO) $(NO_UNDEFINED)
xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS) $(PERL_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-perl-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-perl-exceptions

xapian_wrap.cc Xapian.pm: $(SWIG_sources) util.i except.i
	-rm -f Search Xapian
	ln -s . Search
	ln -s lib Xapian
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -perl -module Search::Xapian -proxy -const \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	mv xapian_wrap_d.tmp xapian_wrap.d

-include xapian_wrap.d

CLEANFILES = $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
