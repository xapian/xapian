## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(PHP4) -d extension_dir=.libs

## Test programs to be run
TESTS = smoketest.php

EXTRA_DIST = util.i extra.i $(TESTS)
DIST_SUBDIRS = docs
SUBDIRS = docs

phpextdir = @PHP_EXTENSION_DIR@

phpext_LTLIBRARIES = xapian.la

# Remove the .la file - xapian.la is never linked against (it's a module)
# and PHP4 doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(phpextdir)/xapian.la

AM_CPPFLAGS = @PHP_INC@
AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) @SWIG_CXXFLAGS@
xapian_la_LDFLAGS = -avoid-version -module
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc php_xapian.h

xapian_wrap.cc php_xapian.h xapian.php: ../xapian.i util.i extra.i
	$(SWIG) -I/usr/include -I. -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -php4 -noproxy -o xapian_wrap.cc $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES) xapian.php
