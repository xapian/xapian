## Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; also so generated files in EXTRA_DIST work
# Need 1.6.3 for tests/internaltest to build with Solaris make
AUTOMAKE_OPTIONS = 1.6.3

if BUILD_BACKEND_REMOTE
remote_subdirs = bin
LIBNET_LA = net/libnet.la
endif

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include getopt testsuite autoconf common \
          api backends matcher languages $(remote_subdirs) \
	  . bin m4 docs tests extra
# Disabled for now: bindings (before extra)

DIST_SUBDIRS = include getopt testsuite autoconf common \
               api backends matcher languages net \
	       . bin m4 docs tests extra
# Disabled for now: bindings (before extra)

bin_SCRIPTS = xapian-config

CLEANFILES = xapian-config

EXTRA_DIST = ChangeLog HACKING PLATFORMS xapian.spec

lib_LTLIBRARIES = libxapian.la

# This line is present because libtool requires at least one source file in
# a library (due to limitations on some operating systems).
libxapian_la_SOURCES = dummy.c
EXTRA_libxapian_la_SOURCES =

# FIXME:
# doesn't really work as we'd hope as we get to play with the mangled symbols
#libxapian_la_LDFLAGS = -export-symbols-regex 'Xapian'

libxapian_la_LIBADD = common/libcommon.la backends/libbackend.la \
 matcher/libmatcher.la languages/liblanguages.la api/libapi.la $(LIBNET_LA)

xapian-config: xapian-config.nodep libxapian.la
	@echo "Creating $@"
	export dependency_libs; \
	. $(top_builddir)/libxapian.la ; \
	sed "s%@dependency_libs@%`echo \"$$dependency_libs\"|sed 's/[^ ]*.la\>//g;s/  */ /g'`%" < xapian-config.nodep > tmp-$@
	-rm -f $@
	mv tmp-$@ $@
