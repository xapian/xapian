## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 subdir-objects
ACLOCAL_AMFLAGS = -I m4

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I../$(top_srcdir)/xapian-core/include

# Order is relevant: when building, tests must be after ".".
SUBDIRS = . docs # tests

AM_CXXFLAGS += $(XAPIAN_CXXFLAGS)

noinst_HEADERS =
BUILT_SOURCES =
noinst_LTLIBRARIES =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
bin_PROGRAMS =
EXTRA_PROGRAMS =

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

EXTRA_DIST = ChangeLog

lib_LTLIBRARIES = libxapianletor.la
libxapianletor_la_SOURCES = $(lib_src)
libxapianletor_la_LDFLAGS = \
    $(XAPIAN_LDFLAGS) -no-undefined -version-info $(LIBRARY_VERSION_INFO)

libxapianletor_la_LIBADD = $(XAPIAN_LIBS)

lib_src =

if !MAINTAINER_NO_DOCS
dist_man_MANS =
endif

include bin/Makefile.mk
include include/Makefile.mk

libxapianletor_la_LIBADD += $(LIBSVM_LIBS)

noinst_HEADERS +=\
	common/safeunistd.h\
	common/str.h\
	common/stringutils.h\
	letor_internal.h\
	feature.h\
	feature_vector.h\
	feature_manager.h\
	ranklist.h\
	ranker.h\
	svmranker.h\
    normalizer.h\
    default_normalizer.h\
    stringhelper.h\
    feature_selector.h


lib_src +=\
	letor.cc\
	letor_internal.cc\
	feature.cc\
	feature_vector.cc\
	feature_manager.cc\
	ranklist.cc\
	svmranker.cc\
    default_normalizer.cc\
    stringhelper.cc\
    feature_selector.cc


DISTCHECK_CONFIGURE_FLAGS = "XAPIAN_CONFIG=$(XAPIAN_CONFIG)"
