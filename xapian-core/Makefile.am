## Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CXXFLAGS, etc; also so generated files in EXTRA_DIST work
# Need 1.6.3 for tests/internaltest to build with Solaris make
# 1.8.5 contains a lot of fixes over 1.6.3
AUTOMAKE_OPTIONS = 1.8.5

if BUILD_BACKEND_REMOTE
remote_subdirs = net
LIBNET_LA = net/libnet.la
endif

# Order is relevant: when building, tests must be last.
# FIXME: when installing, should we install the xapian-config script and
# xapian.m4 last, in case of failure (disk full for example)?
SUBDIRS = include getopt common \
          api backends matcher languages $(remote_subdirs) \
	  extra . testsuite bin examples docs tests

DIST_SUBDIRS = include getopt common \
               api backends matcher languages net \
	       extra . bin examples docs testsuite tests

bin_SCRIPTS = xapian-config

CLEANFILES = xapian-config

EXTRA_DIST = ChangeLog HACKING PLATFORMS xapian.spec m4/xapian.m4 \
 autoconf/rjb_find_stlport.m4 autoconf/definedir.m4 autoconf/type_socklen_t.m4


# xapian.m4 contains the XO_LIB_XAPIAN autoconf macro.
m4datadir = $(datadir)/aclocal
m4data_DATA = m4/xapian.m4

lib_LTLIBRARIES = libxapian.la

# libtool requires at least one source file in a library (due to limitations on
# some operating systems).  Make it a C++ file so that automake knows to link
# the library as C++.
libxapian_la_SOURCES = dummy.cc
EXTRA_libxapian_la_SOURCES =

libxapian_la_LDFLAGS = @XAPIAN_LDFLAGS@ -no-undefined -version-info $(LIBRARY_VERSION_INFO)

# FIXME:
# doesn't really work as we'd hope as we get to play with the mangled symbols
#libxapian_la_LDFLAGS = -export-symbols-regex 'Xapian'

libxapian_la_LIBADD = common/libcommon.la backends/libbackend.la \
 matcher/libmatcher.la languages/liblanguages.la api/libapi.la $(LIBNET_LA) \
 extra/libqueryparser.la
