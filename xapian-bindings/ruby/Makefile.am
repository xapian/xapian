## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(RUBY) -I.libs

## Test programs to be run
TESTS = smoketest.rb

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h

EXTRA_DIST = util.i $(TESTS) $(BUILT_SOURCES)

SUBDIRS = docs

rubylibdir = $(RUBY_LIB)
rubylib_DATA = xapian.rb

rubylibarchdir = $(RUBY_LIB_ARCH)
rubylibarch_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Ruby doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.la

AM_CPPFLAGS = -I$(RUBY_INC)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
_xapian_la_SOURCES = xapian_wrap.cc xapian_wrap.h
_xapian_la_LIBADD = $(XAPIAN_LIBS)

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h: ../xapian.i util.i
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -feature _xapian -ruby -o xapian_wrap.cc $(srcdir)/../xapian.i
endif

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
