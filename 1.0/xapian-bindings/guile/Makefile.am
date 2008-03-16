## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

EXTRA_DIST = util.i $(BUILT_SOURCES)

noinst_LTLIBRARIES = xapianc.la

AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
xapianc_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_xapianc_la_SOURCES = xapian_wrap.cc
xapianc_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
xapian_wrap.cc: $(SWIG_sources) util.i
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -guile \
	    -o xapian_wrap.cc $(SWIG_mainsource)

CLEANFILES = $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
