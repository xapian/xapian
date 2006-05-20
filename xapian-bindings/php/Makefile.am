## Process this file with automake to produce Makefile.in

abs_builddir = @abs_builddir@

# Use -n to disable use of the system php.ini (which might prevent php from
# using dl() which would make our test fail).  It would be nicer to use "-c"
# and provide an alternative php.ini but with "-c" it seems PHP still loads the
# system.ini first (at least with PHP 4.3.10).
TESTS_ENVIRONMENT = $(PHP) -n -d safe_mode=off -d enable_dl=on -d extension_dir=$(abs_builddir)/.libs

## Test programs to be run
TESTS = smoketest.php

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

EXTRA_DIST = util.i xapian.php $(TESTS)
SUBDIRS = docs

phpextdir = $(PHP_EXTENSION_DIR)

phpext_LTLIBRARIES = xapian.la

# Remove the .la file - xapian.la is never linked against (it's a module)
# and PHP doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(phpextdir)/xapian.la

AM_CPPFLAGS = $(PHP_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
xapian_la_SOURCES = xapian_wrap.cc php_xapian.h
xapian_la_LIBADD = $(XAPIAN_LIBS) $(PHP_LIBS)

BUILT_SOURCES = xapian_wrap.cc php_xapian.h

if MAINTAINER_MODE
xapian_wrap.cc php_xapian.h xapian.php: ../xapian.i util.i
	$(SWIG) -I$(srcdir) $(SWIG_FLAGS) -c++ -php4 -noproxy -o xapian_wrap.cc $(srcdir)/../xapian.i
	: #$(SWIG) -I$(srcdir) $(SWIG_FLAGS) -c++ -php4 -prefix Xapian -o xapian_wrap.cc $(srcdir)/../xapian.i
endif

MAINTAINERCLEANFILES = $(BUILT_SOURCES) xapian.php
