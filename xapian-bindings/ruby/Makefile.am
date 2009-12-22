## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = $(RUBY) -I.libs -I$(srcdir)

## Test programs to be run
TESTS = smoketest.rb

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h

EXTRA_DIST = util.i $(TESTS) $(BUILT_SOURCES)

SUBDIRS = . docs

rubylibdir = $(RUBY_LIB)
dist_rubylib_DATA = xapian.rb

rubylibarchdir = $(RUBY_LIB_ARCH)
rubylibarch_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Ruby doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.$(RUBY_DLEXT)

AM_CPPFLAGS = -I$(RUBY_INC) -I$(RUBY_INC_ARCH)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module -shrext ".$(RUBY_DLEXT)" $(NO_UNDEFINED)
_xapian_la_SOURCES = xapian_wrap.cc xapian_wrap.h
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(RUBY_LIBS)

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h: xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	-rm -f xapian_wrap.ccT xapian_wrap.ccU
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -initname _xapian -ruby \
	    -o xapian_wrap.ccT -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	## In Ruby 1.9, there's a memoisation optimisation wrapper for
	## rb_intern under GCC which breaks SWIG's usage so disable this so
	## the code compiles.
	awk '{print} /#include <ruby\.h>/{print "#undef rb_intern\n"}' xapian_wrap.ccT > xapian_wrap.ccU
	rm xapian_wrap.ccT
	mv xapian_wrap.ccU xapian_wrap.cc
	$(PERL) -pe 's!xapian_wrap.ccT:!xapian_wrap.stamp:!' xapian_wrap_d.tmp > xapian_wrap_d2.tmp
	mv xapian_wrap_d2.tmp xapian_wrap.d
	rm xapian_wrap_d.tmp
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES = $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif

# Compatibility wrapper which configure creates when required.
DISTCLEANFILES = rubyio.h
