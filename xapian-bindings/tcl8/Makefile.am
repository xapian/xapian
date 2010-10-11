## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = \
 srcdir="$(srcdir)" LIBTOOL="$(LIBTOOL)" TCLSH="$(TCLSH)" $(srcdir)/run-tcl-test

## Test programs to be run
TESTS = smoketest.tcl

EXTRA_DIST = except.i util.i $(TESTS) $(BUILT_SOURCES) run-tcl-test runtest.tcl

tcllibdir = @TCL_LIB@/xapian@VERSION@

tcllib_LTLIBRARIES = xapian.la
tcllib_DATA = pkgIndex.tcl

# Remove the .la file - xapian.la is never linked against (it's a module)
# and TCL doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(tcllibdir)/xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	eval `grep '^dlname=' $(tcllib_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(tcllibdir)/"$$dlname"

AM_CPPFLAGS = @TCL_CPPFLAGS@
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS) $(TCL_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
xapian_wrap.cc: $(SWIG_sources) except.i util.i
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -tcl8 -namespace -pkgversion @VERSION_NO_SNAPSHOT@ \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	mv xapian_wrap_d.tmp xapian_wrap.d

-include xapian_wrap.d
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/tcl8/examples
dist_exampledata_DATA = \
	docs/examples/simpleexpand.tcl \
	docs/examples/simpleindex.tcl \
	docs/examples/simplesearch.tcl

docdatadir = $(docdir)/tcl8
dist_docdata_DATA = docs/index.html
