## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i

noinst_LTLIBRARIES = xapianc.la

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) @SWIG_CXXFLAGS@
xapianc_la_LDFLAGS = -avoid-version -module
nodist_xapianc_la_SOURCES = xapian_wrap.cc
xapianc_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

xapian_wrap.cc: ../xapian.i util.i
	$(SWIG) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -guile -o $@ $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
