## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = \
 srcdir="$(srcdir)" LIBTOOL="$(LIBTOOL)" TCLSH="$(TCLSH)" $(srcdir)/run-tcl-test

## Test programs to be run
TESTS = smoketest.tcl

EXTRA_DIST = except.i util.i $(TESTS) $(BUILT_SOURCES) run-tcl-test runtest.tcl
SUBDIRS = docs

tcllibdir = @TCL_LIB@/../xapian@VERSION@

tcllib_LTLIBRARIES = xapian.la
tcllib_DATA = pkgIndex.tcl

# Remove the .la file - xapian.la is never linked against (it's a module)
# and TCL doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(tcllibdir)/xapian.la

AM_CPPFLAGS = @TCL_CPPFLAGS@ -DUSE_TCL_STUBS
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

pkgIndex.tcl: Makefile.am
	echo 'package ifneeded xapian @VERSION_NO_SNAPSHOT@ [list load [file join $$dir xapian[info sharedlibextension]]]' > pkgIndex.tcl

if MAINTAINER_MODE
xapian_wrap.cc: $(SWIG_sources) except.i util.i
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -tcl8 -namespace -pkgversion @VERSION_NO_SNAPSHOT@ \
	    -o xapian_wrap.cc $(SWIG_mainsource)

CLEANFILES = $(BUILT_SOURCES) pkgIndex.tcl
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
CLEANFILES = pkgIndex.tcl
endif
