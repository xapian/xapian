## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/extra
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig

noinst_LTLIBRARIES = xapianc.la

# swig generated code gives lots of (non-harmful) warnings with -W or -Wunused
AM_CXXFLAGS = -Wall -Wno-unused
xapianc_la_LDFLAGS = -avoid-version -module
nodist_xapianc_la_SOURCES = xapian_wrap.cc
xapianc_la_LIBADD = ../../libxapian.la ../../extra/libomqueryparser.la

BUILT_SOURCES = xapian_wrap.cc

xapian_wrap.cc: ../swig/xapian.i ../swig/om_util.i ../swig/om_util_guile.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -guile -shadow -o $@ $(srcdir)/../swig/xapian.i

CLEANFILES = $(BUILT_SOURCES)
