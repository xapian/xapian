## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(TCLSH)

## Test programs to be run
TESTS = smoketest.tcl

EXTRA_DIST = util.i extra.i $(TESTS)

tcllibdir = @TCL_LIB@/../xapian@VERSION@

tcllib_LTLIBRARIES = xapian.la
tcllib_DATA = pkgIndex.tcl

# Remove the .la file - xapian.la is never linked against (it's a module)
# and TCL doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(tcllibdir)/xapian.la

AM_CPPFLAGS = @TCL_CPPFLAGS@ -DUSE_TCL_STUBS
AM_CXXFLAGS = @SWIG_CXXFLAGS@
xapian_la_LDFLAGS = -avoid-version -module
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

xapian_wrap.cc: ../xapian.i util.i extra.i
	$(SWIG) -namespace -pkgversion @VERSION@ -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -tcl8 -o $@ $(srcdir)/../xapian.i

pkgIndex.tcl: Makefile.am
	echo 'package ifneeded xapian @VERSION@ [list load [file join $$dir xapian[info sharedlibextension]]]' > pkgIndex.tcl

CLEANFILES = $(BUILT_SOURCES) pkgIndex.tcl
