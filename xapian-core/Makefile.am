### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

SUBDIRS = include backends matcher stemmer indexer . \
          gui tests ferretfx docs

EXTRA_DIST = INSTALL

lib_LTLIBRARIES = libom.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libom_la_SOURCES = dummy.c

libom_la_LIBADD = \
 backends/libbackend.la \
 matcher/libmatcher.la \
 stemmer/libstem.la \
 indexer/libindexer.la
