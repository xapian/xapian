## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

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

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.$(RUBY_DLEXT)

AM_CPPFLAGS = -I$(RUBY_INC)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module -shrext ".$(RUBY_DLEXT)" $(NO_UNDEFINED)
_xapian_la_SOURCES = xapian_wrap.cc xapian_wrap.h
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(RUBY_LIBS)

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h: xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -initname _xapian -ruby \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pe 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp > xapian_wrap_d2.tmp
	mv xapian_wrap_d2.tmp xapian_wrap.d
	rm xapian_wrap_d.tmp
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES = $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
