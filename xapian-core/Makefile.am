## Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; also so generated files in EXTRA_DIST work
# Need 1.6.3 for tests/internaltest to build with Solaris make
AUTOMAKE_OPTIONS = 1.6.3

if BUILD_BACKEND_REMOTE
remote_subdirs = net
LIBNET_LA = net/libnet.la
endif

# Order is relevant: when building, tests must be last.
# FIXME: when installing, should we install the xapian-config script and
# xapian.m4 last, in case of failure (disk full for example)?
SUBDIRS = include getopt common \
          api backends matcher languages $(remote_subdirs) \
	  . testsuite bin docs tests extra

DIST_SUBDIRS = include getopt common \
               api backends matcher languages net \
	       . bin docs testsuite tests extra

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

# Extract from the libtool info manual:
#
# Here are a set of rules to help you update your library version information:
#
# 1. Start with version information of `0:0:0' for each libtool library.
#
# 2. Update the version information only immediately before a public
#    release of your software.  More frequent updates are unnecessary,
#    and only guarantee that the current interface number gets larger
#    faster.
#
# 3. If the library source code has changed at all since the last
#    update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
# 4. If any interfaces have been added, removed, or changed since the
#    last update, increment CURRENT, and set REVISION to 0.
#
# 5. If any interfaces have been added since the last public release,
#    then increment AGE.
#
# 6. If any interfaces have been removed since the last public release
#    then set AGE to 0.
#
# 0:0:0 0.7.2 (and anything before)
# 0:1:0 0.7.3
# 1:0:0 0.7.4 (Xapian::ESet now has reference counted internals)
# 1:1:1 0.7.5 (Xapian::{Term,Posting,Position}Iterator default ctors added)
# 2:0:0 0.8.0 (TradWeight ctor explicit; Xapian::Document::add_term() added)
libxapian_la_LDFLAGS = @XAPIAN_LDFLAGS@ -no-undefined -version-info 2:0:0

# FIXME:
# doesn't really work as we'd hope as we get to play with the mangled symbols
#libxapian_la_LDFLAGS = -export-symbols-regex 'Xapian'

libxapian_la_LIBADD = common/libcommon.la backends/libbackend.la \
 matcher/libmatcher.la languages/liblanguages.la api/libapi.la $(LIBNET_LA)
