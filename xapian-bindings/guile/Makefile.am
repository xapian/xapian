## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i $(BUILT_SOURCES)

noinst_LTLIBRARIES = xapianc.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapianc_la_LDFLAGS = -avoid-version -module -no-undefined
nodist_xapianc_la_SOURCES = xapian_wrap.cc
xapianc_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
xapian_wrap.cc: ../xapian.i util.i extra.i
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -guile -o $@ $(srcdir)/../xapian.i
endif

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
