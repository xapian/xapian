## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

## Test programs to be run
TESTS = smoketest.tcl
AM_TESTS_ENVIRONMENT = \
	srcdir='$(srcdir)' ;\
	TCLSH='$(TCLSH)' ;\
	TCL_SHLIB_EXT='$(TCL_SHLIB_EXT)' ;\
	_libs="$(_libs)" ;\
	export srcdir ;\
	export TCLSH ;\
	export TCL_SHLIB_EXT ;\
	export _libs ;

LOG_COMPILER = $(srcdir)/run-tcl-test

installcheck-local:
	$(MAKE) check LOG_COMPILER='$(TCLSH)' AM_TESTS_ENVIRONMENT=

SWIG_GENERATED = xapian_wrap.cc

EXTRA_DIST = tcl.i except.i \
	generate-tcl-exceptions \
	run-tcl-test runtest.tcl \
	$(TESTS) $(SWIG_GENERATED)

BUILT_SOURCES = $(SWIG_GENERATED)

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
	rm -f $(DESTDIR)$(tcllibdir)/xapian$(TCL_SHLIB_EXT)

AM_CPPFLAGS = @TCL_CPPFLAGS@
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module -shrext '$(TCL_SHLIB_EXT)' $(NO_UNDEFINED)
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS) $(TCL_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-tcl-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-tcl-exceptions

SWIG_FLAGS += -MD -MP

xapian_wrap.cc: except.i
	$(SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
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
dist_docdata_DATA = docs/index.html

if DOCUMENTATION_RULES
BUILT_SOURCES += docs/index.html

.rst.html:
	-case "$@" in */*) d=`echo "$@"|sed 's,/[^/]*$$,,'`; $(MKDIR_P) "$$d" ;; esac
	$(RST2HTML) --exit-status=warning $< $@
endif
