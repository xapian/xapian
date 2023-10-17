## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

abs_builddir = @abs_builddir@

## Test programs to be run
TESTS = smoketest.php
LOG_COMPILER = \
    $(LIBTOOL) -dlopen xapian.la --mode=execute $(MACOS_SIP_HACK_ENV) $(PHP)
# Use -n to disable use of the system php.ini to avoid custom configuration
# settings there which might interfere with our tests.
AM_LOG_FLAGS = -n -d extension_dir='$(abs_builddir)/$(_libs)' -d extension=xapian

installcheck-local:
	$(MAKE) check AM_LOG_FLAGS=

SWIG_GENERATED =\
	php/xapian_wrap.cc\
	php/xapian_wrap.h\
	php/php_xapian.h

EXTRA_DIST = php.i except.i \
	generate-php-exceptions \
	add-php-ref-handling \
	docs/index.rst \
	$(TESTS) $(SWIG_GENERATED)

BUILT_SOURCES = $(SWIG_GENERATED)

phpextdir = $(PHP_EXTENSION_DIR)

phpext_LTLIBRARIES = xapian.la

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
xapian_la_SOURCES = php/xapian_wrap.cc php/xapian_wrap.h
xapian_la_LIBADD = $(XAPIAN_LIBS) $(PHP_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-php-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-php-exceptions

# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

stamp = php/xapian_wrap.stamp
SWIG_FLAGS += -MD -MP -MT $(stamp)

php/xapian_wrap.cc php/xapian_wrap.h php/php_xapian.h: $(stamp)
	$(make_many_locked)
$(stamp): except.i add-php-ref-handling
	$(MKDIR_P) php
	$(multitarget_begin)
	$(SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -DPACKAGE_VERSION='"$(PACKAGE_VERSION)"' \
	    -php -prefix Xapian -outdir php \
	    -o php/xapian_wrap.cc '$(srcdir)/'php.i
	$(PERL) '$(srcdir)/'add-php-ref-handling php/xapian_wrap.cc \
	    > php/xapian_wrap.tmp
	mv php/xapian_wrap.tmp php/xapian_wrap.cc
	$(multitarget_end)

-include php/xapian_wrap.d

CLEANFILES = $(stamp)
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/php/examples
dist_exampledata_DATA =\
	docs/examples/simplesearch.php8\
	docs/examples/simpleindex.php8\
	docs/examples/simpleexpand.php8\
	docs/examples/simplematchdecider.php8

docdatadir = $(docdir)/php
dist_docdata_DATA = docs/index.html

if DOCUMENTATION_RULES
BUILT_SOURCES += docs/index.html

.rst.html:
	-case "$@" in */*) d=`echo "$@"|sed 's,/[^/]*$$,,'`; $(MKDIR_P) "$$d" ;; esac
	$(RST2HTML) --exit-status=warning $< $@
endif
