## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

## Test programs to be run
TESTS = smoketest.tcl
AM_TESTS_ENVIRONMENT = \
	srcdir='$(srcdir)' ;\
	LIBTOOL='$(LIBTOOL)' ;\
	TCLSH='$(TCLSH)' ; \
	export srcdir ;\
	export LIBTOOL ;\
	export TCLSH ;
LOG_COMPILER = $(srcdir)/run-tcl-test

EXTRA_DIST = tcl.i except.i \
	generate-tcl-exceptions \
	run-tcl-test runtest.tcl \
	$(TESTS) $(BUILT_SOURCES)

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
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-tcl-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-tcl-exceptions

RUN_SWIG = $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

xapian_wrap.cc: except.i
	$(RUN_SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -tcl8 -namespace -pkgversion '@VERSION_NO_SNAPSHOT@' \
	    -o xapian_wrap.cc '$(srcdir)/'tcl.i

-include xapian_wrap.d
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/tcl8/examples
dist_exampledata_DATA = \
	docs/examples/simpleexpand.tcl \
	docs/examples/simpleindex.tcl \
	docs/examples/simplesearch.tcl

docdatadir = $(docdir)/tcl8

if DOCUMENTATION_RULES
docdata_DATA = docs/index.html
BUILT_SOURCES += docs/index.html

.rst.html:
	-case "$@" in */*) d=`echo "$@"|sed 's,/[^/]*$$,,'`; test -d "$$d" || mkdir "$$d" ;; esac
	$(RST2HTML) --exit-status=warning $< $@
endif
