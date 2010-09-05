## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = prove -I.libs -Ilib

## Test programs to be run
TESTS = t/*.t

BUILT_SOURCES = xapian_wrap.cc

EXTRA_DIST = util.i except.i index.html $(TESTS) $(BUILT_SOURCES)

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

BUILT_SOURCES += index.html
index.html: Xapian.pm
	pod2html Xapian.pm > docs/index.html

xapian_wrap.cc Xapian.pm: xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i except.i
	$(multitarget_begin)
	-rm -f Search Xapian
	ln -s . Search
	ln -s lib Xapian
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -perl -module Search::Xapian -proxy -const \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	mv xapian_wrap_d.tmp xapian_wrap.d
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES = $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
