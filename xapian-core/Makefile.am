### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include common \
          backends matcher languages indexer . m4 \
	  docs tests

EXTRA_DIST = AUTHORS INSTALL README COPYING ChangeLog buildall HACKING TODO

lib_LTLIBRARIES = libomus.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libomus_la_SOURCES = dummy.c

libomus_la_LIBADD = \
 backends/libcommon.la \
 backends/libbackend.la \
 matcher/libmatcher.la \
 languages/liblanguages.la \
 indexer/libindexer.la

bin_SCRIPTS = libomus-config
