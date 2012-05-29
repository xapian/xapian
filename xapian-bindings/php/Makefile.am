## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

abs_builddir = @abs_builddir@

# Use -n to disable use of the system php.ini (which might prevent php from
# using dl() which would make our test fail).  It would be nicer to use "-c"
# and provide an alternative php.ini but with "-c" it seems PHP still loads the
# system.ini first (at least with PHP 4.3.10).
TESTS_ENVIRONMENT = $(PHP) -n -d safe_mode=off -d enable_dl=on \
	-d extension_dir='$(abs_builddir)/.libs' \
	-d include_path='php5$(PHP_PATH_SEPARATOR)$(srcdir)/php5'

## Test programs to be run
TESTS = smoketest.php

BUILT_SOURCES =\
	php5/xapian_wrap.cc\
	php5/xapian_wrap.h\
	php5/php_xapian.h\
	php5/xapian.php

EXTRA_DIST = php.i except.i \
	generate-php-exceptions \
	add-php-type-hints \
	$(TESTS) $(BUILT_SOURCES)

phpextdir = $(PHP_EXTENSION_DIR)

phpext_LTLIBRARIES = xapian.la

# This location is correct for Debian, but in general there doesn't seem
# to be a suitable location which is guaranteed to be in the include_path
# by default.
phpincdir = $(datadir)/php$(PHP_MAJOR_VERSION)
phpinc_DATA = php5/xapian.php

# Remove the .la file - xapian.la is never linked against (it's a module)
# and PHP doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(phpextdir)/xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	eval `grep '^dlname=' $(phpext_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(phpextdir)/"$$dlname"

AM_CPPFLAGS = $(PHP_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
xapian_la_SOURCES = php5/xapian_wrap.cc php5/xapian_wrap.h
xapian_la_LIBADD = $(XAPIAN_LIBS) $(PHP_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-php-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-php-exceptions

# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

php5/xapian_wrap.cc php5/xapian_wrap.h php5/php_xapian.h php5/xapian.php: php5/xapian_wrap.stamp
	@stamp=php5/xapian_wrap.stamp; $(make_many_locked)
php5/xapian_wrap.stamp: except.i add-php-type-hints
	-test -d php5 || mkdir php5
	$(multitarget_begin)
	$(SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -php5 -prefix Xapian -outdir php5 \
	    -o php5/xapian_wrap.cc -MD -MF xapian_wrap_d.tmp '$(srcdir)/'php.i
	$(PERL) -pi -e 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp
	mv xapian_wrap_d.tmp xapian_wrap.d
	$(PERL) '$(srcdir)/'add-php-type-hints php5/xapian.php > php5/xapian.tmp
	mv php5/xapian.tmp php5/xapian.php
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES = php5/xapian_wrap.stamp
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/php/examples
dist_exampledata_DATA =\
	docs/examples/simplesearch.php5\
	docs/examples/simpleindex.php5\
	docs/examples/simpleexpand.php5\
	docs/examples/simplematchdecider.php5

docdatadir = $(docdir)/php
dist_docdata_DATA = docs/index.html
