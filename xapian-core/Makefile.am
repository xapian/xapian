## Process this file with automake to produce Makefile.in

if BUILD_BACKEND_NET
BUILDSUBDIR_NET1 = net
BUILDSUBDIR_NET2 = netprogs
endif

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include testsuite autoconf common \
          backends api matcher languages $(BUILDSUBDIR_NET1) indexer . m4 \
	  docs $(BUILDSUBDIR_NET2) tests bindings

EXTRA_DIST = Makefile_common.am ChangeLog buildall HACKING TODO PLATFORMS

lib_LTLIBRARIES = libomus.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libomus_la_SOURCES = dummy.c
EXTRA_libomus_la_SOURCES =

libomus_la_LIBADD = \
 common/libcommon.la \
 backends/libbackend.la \
 matcher/libmatcher.la \
 languages/liblanguages.la \
 indexer/libindexer.la \
 api/libapi.la \
 net/libnet.la

bin_SCRIPTS = libomus-config

libomus-config: libomus-config.nodep libomus.la
	@echo "Creating $@"
	@export dependency_libs; \
	. libomus.la ; \
	cat $< | sed "s%@dependency_libs@%$${dependency_libs}%" > $@
