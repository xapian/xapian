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
## We need to rename the file to have the extension Ruby expects on certain
## platforms (e.g. Mac OS X).
	$(LIBTOOL) --config > libtoolconfig.tmp
	. ./libtoolconfig.tmp; module=yes; eval shlibext=$$shrext_cmds; \
	  test ".$(RUBY_DLEXT)" = "$$shlibext" || mv $(DESTDIR)$(rubylibarchdir)/_xapian$$shlibext $(DESTDIR)$(rubylibarchdir)/_xapian.$(RUBY_DLEXT)
	rm -f libtoolconfig.tmp

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.  Also, the extension may not match
# what libtool expects.
uninstall-local:
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.$(RUBY_DLEXT)

AM_CPPFLAGS = -I$(RUBY_INC)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
_xapian_la_SOURCES = xapian_wrap.cc xapian_wrap.h
_xapian_la_LIBADD = $(XAPIAN_LIBS)

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h: xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -initname _xapian -ruby \
	    -o xapian_wrap.cc $(SWIG_mainsource)
	$(multitarget_end)

CLEANFILES = $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
