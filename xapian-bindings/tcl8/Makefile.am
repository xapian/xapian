## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(TCLSH)

## Test programs to be run
TESTS = smoketest.tcl

EXTRA_DIST = util.i $(TESTS) $(BUILT_SOURCES)
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
xapian_la_LDFLAGS = -avoid-version -module -no-undefined
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
xapian_wrap.cc: ../xapian.i util.i
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -tcl8 -namespace -pkgversion @VERSION@ -o $@ $(srcdir)/../xapian.i
endif

pkgIndex.tcl: Makefile.am
	echo 'package ifneeded xapian @VERSION@ [list load [file join $$dir xapian[info sharedlibextension]]]' > pkgIndex.tcl

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
CLEANFILES = pkgIndex.tcl
