## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

abs_builddir = @abs_builddir@

## Test programs to be run
TESTS = smoketest.php
LOG_COMPILER = \
    $(LIBTOOL) -dlopen xapian.la --mode=execute $(OSX_SIP_HACK_ENV) $(PHP7)
# Use -n to disable use of the system php.ini to avoid custom configuration
# settings there which might interfere with our tests.
AM_LOG_FLAGS = -n -d extension_dir='$(abs_builddir)/.libs' -d extension=xapian \
	-d include_path='php7$(PHP7_PATH_SEPARATOR)$(srcdir)/php7'

installcheck-local:
	$(MAKE) check AM_LOG_FLAGS=

SWIG_GENERATED =\
	php7/xapian_wrap.cc\
	php7/xapian_wrap.h\
	php7/php_xapian.h\
	php7/xapian.php

EXTRA_DIST = php.i except.i \
	generate-php-exceptions \
	add-php-ref-handling \
	add-php-type-hints \
	docs/index.rst \
	$(TESTS) $(SWIG_GENERATED)

BUILT_SOURCES = $(SWIG_GENERATED)

phpextdir = $(PHP7_EXTENSION_DIR)

phpext_LTLIBRARIES = xapian.la

# This location is correct for Debian, but in general there doesn't seem
# to be a suitable location which is guaranteed to be in the include_path
# by default.
phpincdir = $(datadir)/php
phpinc_DATA = php7/xapian.php

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

AM_CPPFLAGS = $(PHP7_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
xapian_la_SOURCES = php7/xapian_wrap.cc php7/xapian_wrap.h
xapian_la_LIBADD = $(XAPIAN_LIBS) $(PHP7_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-php-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-php-exceptions

# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

stamp = php7/xapian_wrap.stamp
RUN_SWIG = stamp='$(stamp)' $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

php7/xapian_wrap.cc php7/xapian_wrap.h php7/php_xapian.h php7/xapian.php: $(stamp)
	$(make_many_locked)
$(stamp): except.i add-php-ref-handling add-php-type-hints
	$(MKDIR_P) php7
	$(multitarget_begin)
	$(RUN_SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -php7 -prefix Xapian -outdir php7 \
	    -o php7/xapian_wrap.cc '$(srcdir)/'php.i
	$(PERL) '$(srcdir)/'add-php-ref-handling php7/xapian.php | \
	    $(PERL) '$(srcdir)/'add-php-type-hints > php7/xapian.tmp
	mv php7/xapian.tmp php7/xapian.php
	$(multitarget_end)

-include php7/xapian_wrap.d

CLEANFILES = $(stamp)
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/php/examples
dist_exampledata_DATA =\
	docs/examples/simplesearch.php7\
	docs/examples/simpleindex.php7\
	docs/examples/simpleexpand.php7\
	docs/examples/simplematchdecider.php7

docdatadir = $(docdir)/php
dist_docdata_DATA = docs/index.html

if DOCUMENTATION_RULES
BUILT_SOURCES += docs/index.html

.rst.html:
	-case "$@" in */*) d=`echo "$@"|sed 's,/[^/]*$$,,'`; $(MKDIR_P) "$$d" ;; esac
	$(RST2HTML) --exit-status=warning $< $@
endif
