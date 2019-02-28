## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

GO_INC = /usr/include
GO_LIBS =
GO_SO = .so

## Test programs to be run
TESTS = smoketest.go
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)' ;\
	export abs_builddir ;\
	export srcdir ;
LOG_COMPILER = go test xapian.go

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h xapian.go

EXTRA_DIST = go.i util.i extra.i extracomments.i except.i \
	generate-go-exceptions \
	smoketest.go \
	$(TESTS) $(BUILT_SOURCES)

pkggolibdir = /usr/lib/go-or-something/xapian

pkggolib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Go doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pkggolibdir)/_xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(pkggolibdir)/_xapian$(GO_SO)

AM_CPPFLAGS = -I$(GO_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module -shrext "$(GO_SO)" $(NO_UNDEFINED)
_xapian_la_SOURCES = xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(GO_LIBS)

xapian/_xapian$(GO_SO): _xapian.la
	-test -d xapian || mkdir xapian
	$(LIBTOOL) --config > libtoolconfig.tmp
## ksh requires a path on the sourced file.
	. ./libtoolconfig.tmp; cp $$objdir/_xapian$(GO_SO) xapian
	rm -f libtoolconfig.tmp

CLEANFILES = \
    xapian/_xapian$(GO_SO)

# Clean the xapian directory which we created, if it's empty, and any
# databases created by test cases.
clean-local:
	-rmdir xapian
	rm -rf db_test_* dbs_replication

if MAINTAINER_MODE
# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-go-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-go-exceptions

stamp = xapian_wrap.stamp
RUN_SWIG = stamp='$(stamp)' $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

xapian_wrap.cc xapian_wrap.h xapian.go xapian_wrap.d: $(stamp)
	$(make_many_locked)
$(stamp): except.i
	$(multitarget_begin)
	$(RUN_SWIG) $(SWIG_WERROR) -I. -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -go -intgosize 32 -o xapian_wrap.cc '$(srcdir)/'go.i
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += xapian_wrap.d $(stamp)
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/go/examples
dist_exampledata_DATA = 

docdatadir = $(docdir)/go
dist_docdata_DATA = 
