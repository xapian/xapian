## Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; also so generated files in EXTRA_DIST work
# Need 1.6.3 for tests/internaltest to build with Solaris make
AUTOMAKE_OPTIONS = 1.6.3

if BUILD_BACKEND_REMOTE
BUILDSUBDIR_REMOTE1 = net
BUILDSUBDIR_REMOTE2 = netprogs
LIBNET_LA = net/libnet.la
endif

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include getopt testsuite autoconf common \
          api backends matcher languages $(BUILDSUBDIR_REMOTE1) \
	  . m4 docs $(BUILDSUBDIR_REMOTE2) tests extra
# Disabled for now: bindings

DIST_SUBDIRS = include testsuite autoconf common \
               api backends matcher languages net \
	       . m4 docs netprogs tests extra getopt
# Disabled for now: bindings

bin_SCRIPTS = xapian-config

CLEANFILES = xapian-config

EXTRA_DIST = ChangeLog HACKING PLATFORMS xapian.spec

lib_LTLIBRARIES = libxapian.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libxapian_la_SOURCES = dummy.c
EXTRA_libxapian_la_SOURCES =

# FIXME:
# doesn't really work as we'd hope as we get to play with the mangled symbols
#libxapian_la_LDFLAGS = -export-symbols-regex 'Om[A-Z]'

libxapian_la_LIBADD = common/libcommon.la backends/libbackend.la \
 matcher/libmatcher.la languages/liblanguages.la api/libapi.la $(LIBNET_LA)

xapian-config: xapian-config.nodep libxapian.la
	@echo "Creating $@"
	export dependency_libs; \
	. $(top_builddir)/libxapian.la ; \
	sed "s%@dependency_libs@%`echo \"$$dependency_libs\"|sed 's/[^ ]*.la\>//g;s/  */ /g'`%" < xapian-config.nodep > tmp-$@
	-rm -f $@
	mv tmp-$@ $@
