## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include common \
          backends api matcher languages net indexer . m4 \
	  docs tests netprogs

EXTRA_DIST = Makefile_common.am ChangeLog buildall HACKING TODO PLATFORMS

lib_LTLIBRARIES = libomus.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libomus_la_SOURCES = dummy.c

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
	export dependency_libs; \
	. libomus.la ; \
	cat $< | sed "s%@dependency_libs@%$${dependency_libs}%" > $@
