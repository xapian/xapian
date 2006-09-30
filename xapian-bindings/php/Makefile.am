## Process this file with automake to produce Makefile.in

abs_builddir = @abs_builddir@

# Use -n to disable use of the system php.ini (which might prevent php from
# using dl() which would make our test fail).  It would be nicer to use "-c"
# and provide an alternative php.ini but with "-c" it seems PHP still loads the
# system.ini first (at least with PHP 4.3.10).
#
# Pass "-q" option, which suppresses HTTP header output for PHP < 4.3 and is
# ignored for compatibility by more recent versions of PHP.
TESTS_ENVIRONMENT = $(PHP) -q -n -d safe_mode=off -d enable_dl=on -d extension_dir=$(abs_builddir)/.libs

## Test programs to be run
TESTS = smoketest.php

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

# Interleave php4 and php5 as the .cc-s and .h-s are identical (currently
# at least) and gzip will compress this redundancy away if it falls within the
# compressor's window size.
BUILT_SOURCES = php4/xapian_wrap.cc php5/xapian_wrap.cc \
		php4/php_xapian.h php5/php_xapian.h \
		php4/xapian.php php5/xapian.php

EXTRA_DIST = util.i $(TESTS) smoketest4.php smoketest5.php $(BUILT_SOURCES)

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
xapian_la_SOURCES = php$(PHP_MAJOR_VERSION)/xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS) $(PHP_LIBS)

if MAINTAINER_MODE
# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

php4/xapian_wrap.cc php4/php_xapian.h php4/xapian.php: ../xapian.i util.i
	test -d php4 || mkdir php4
	$(SWIG) -I$(srcdir) $(SWIG_FLAGS) -c++ -php4 -noproxy \
	    -outdir php4 -o php4/xapian_wrap.cc $(srcdir)/../xapian.i

php5/xapian_wrap.cc php5/php_xapian.h php5/xapian.php: ../xapian.i util.i
	test -d php5 || mkdir php5
	$(SWIG) -I$(srcdir) $(SWIG_FLAGS) -c++ -php5 -prefix Xapian \
	    -outdir php5 -o php5/xapian_wrap.cc $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
