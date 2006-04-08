## Process this file with automake to produce Makefile.in

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

EXTRA_DIST = util.i $(BUILT_SOURCES)

noinst_LTLIBRARIES = xapianc.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapianc_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_xapianc_la_SOURCES = xapian_wrap.cc
xapianc_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
xapian_wrap.cc: ../xapian.i util.i
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -guile -o $@ $(srcdir)/../xapian.i
endif

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
