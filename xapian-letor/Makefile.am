## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/include
if VPATH_BUILD
# Needed for the generated files
# include/xapian-letor/letor_{error,errordispatch}.h
AM_CPPFLAGS += -I'$(top_builddir)/include'
endif

AM_CXXFLAGS += $(XAPIAN_CXXFLAGS)

# Order is relevant: when building, tests must be after ".".
SUBDIRS = . docs tests

noinst_HEADERS =
BUILT_SOURCES =
noinst_LTLIBRARIES =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
bin_PROGRAMS =
EXTRA_PROGRAMS =

.PHONY: coverage-reconfigure coverage-reconfigure-maintainer-mode coverage-check

# This relies on GNU make (for the pattern rule) but it's just a convenience
# as you can run these check-* targets with any make using:
#
#   cd tests ; make check-FOO
#
# So the pain of maintaining a huge list of target forwarding rules here isn't
# worth it.
#
# Note: We use @PERCENT@ here instead of a literal % to suppress an automake
# portability warning.
check-@PERCENT@:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@

## -O0 - optimisation can result in confusing coverage reports.
## --coverage - inserts code to generate coverage data
## VALGRIND= - generating coverage data is slow enough by itself.
COVERAGE_CONFIGURE = \
    $(srcdir)/configure CXXFLAGS='-O0 --coverage' VALGRIND= \
	$(COVERAGE_CONFIGURE_ARGS)

coverage-reconfigure: distclean
	$(COVERAGE_CONFIGURE)

coverage-reconfigure-maintainer-mode: distclean
	$(COVERAGE_CONFIGURE) --enable-maintainer-mode

coverage-check:
	## GNU find specific.
	find . -name '*.gcda' -delete
	## Don't run under valgrind even if configure was run by hand.
	$(MAKE) $(AM_MAKEFLAGS) check VALGRIND=
	rm -f xapian-letor.lcov
	lcov --capture -d . --quiet --rc 'lcov_branch_coverage=1' --no-external -o xapian-letor.lcov
	rm -rf lcov
	genhtml --demangle-cpp --branch-coverage $(GENHTML_ARGS) -q -p "`pwd`" -t "Test Coverage for xapian-letor `git log -n1 --abbrev-commit --format=%h`" -o lcov xapian-letor.lcov

# We want to clean out build tree paths from the copy of xapianletor-config we
# install, mostly in the interests of facilitating reproducible builds.
bin_SCRIPTS = installable/xapianletor-config

# Create the installable version in a subdirectory.  It's not made executable
# until install time, which hopefully should avoid people trying to use it when
# building against an uninstalled xapian-letor.
installable/xapianletor-config: xapianletor-config
	$(MKDIR_P) installable
	$(SED) '/##UNINSTONLYBEGIN/,/##UNINSTONLYEND/d' xapianletor-config > installable/xapianletor-config.tmp
	mv installable/xapianletor-config.tmp installable/xapianletor-config

CLEANFILES += installable/xapianletor-config

noinst_SCRIPTS = xapianletor-config

DISTCLEANFILES += xapianletor-config

EXTRA_DIST = ChangeLog include/xapian-letor/letor_error.h \
 include/xapian-letor/letor_errordispatch.h generate-exceptions exception_data.pm

cmakedir = $(libdir)/cmake/xapian-letor
cmake_DATA = cmake/xapianletor-config.cmake cmake/xapianletor-config-version.cmake

# Install the m4 file containing the XO_LIB_XAPIANLETOR autoconf macro.
m4datadir = $(datadir)/aclocal
dist_m4data_DATA = m4-macros/xapianletor.m4

lib_LTLIBRARIES = libxapianletor.la
libxapianletor_la_SOURCES = $(lib_src)
libxapianletor_la_LDFLAGS = \
	$(XAPIANLETOR_LDFLAGS) $(NO_UNDEFINED) -version-info $(LIBRARY_VERSION_INFO)

libxapianletor_la_LIBADD = $(XAPIAN_LIBS) libcommon_str.la

lib_src =

if !MAINTAINER_NO_DOCS
dist_man_MANS =
endif

include api/Makefile.mk
include bin/Makefile.mk
include include/Makefile.mk
include feature/Makefile.mk
include ranker/Makefile.mk
include scorer/Makefile.mk

noinst_LTLIBRARIES += libcommon_str.la libgetopt.la
libcommon_str_la_SOURCES = common/str.cc
libgetopt_la_SOURCES = common/getopt.cc

xapianletorinclude_HEADERS += include/xapian-letor/letor_error.h

if MAINTAINER_MODE
BUILT_SOURCES += include/xapian-letor/letor_error.h include/xapian-letor/letor_errordispatch.h

include/xapian-letor/letor_error.h include/xapian-letor/letor_errordispatch.h: generate-exceptions exception_data.pm
	$(PERL) -w -I"$(srcdir)" "$(srcdir)/generate-exceptions"
endif

MAINTAINERCLEANFILES += $(BUILT_SOURCES)

libxapianletor_la_LIBADD += $(LIBSVM_LIBS)

noinst_HEADERS +=\
	common/alignment_cast.h\
	common/append_filename_arg.h\
	common/debuglog.h\
	common/errno_to_string.h\
	common/filetests.h\
	common/fileutils.h\
	common/gnu_getopt.h\
	common/log2.h\
	common/omassert.h\
	common/output.h\
	common/overflow.h\
	common/parseint.h\
	common/safedirent.h\
	common/safefcntl.h\
	common/safesyssocket.h\
	common/safesysstat.h\
	common/safeunistd.h\
	common/str.h\
	common/stringutils.h\
	common/wordaccess.h

DISTCHECK_CONFIGURE_FLAGS = "XAPIAN_CONFIG=$(XAPIAN_CONFIG)"
