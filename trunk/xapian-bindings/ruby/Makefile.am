## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = $(RUBY) -I.libs -I$(srcdir)

## Test programs to be run
TESTS = smoketest.rb

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h

EXTRA_DIST = util.i $(TESTS) $(BUILT_SOURCES)

SUBDIRS = docs

rubylibdir = $(RUBY_LIB)
dist_rubylib_DATA = xapian.rb

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
xapian_wrap.cc xapian_wrap.h: xapian_wrap.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf xapian_wrap.lock xapian_wrap.stamp' 1 2 13 15; \
	  if mkdir xapian_wrap.lock 2>/dev/null; then \
	    rm -f xapian_wrap.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) xapian_wrap.stamp; \
	    rmdir xapian_wrap.lock; \
	  else \
	    while test -d xapian_wrap.lock; do sleep 1; done; \
	    test -f xapian_wrap.stamp; exit $$?; \
	  fi; \
	fi
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -feature _xapian -ruby \
	    -o xapian_wrap.cc $(SWIG_mainsource)
	touch $@

CLEANFILES = $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
