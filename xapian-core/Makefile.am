## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

SUBDIRS = include backends matcher stemmer . tests #gui

lib_LTLIBRARIES = libom.la

libom_la_SOURCES =
libom_la_LIBADD = backends/da/libda.la backends/sleepy/libsleepy.la \
 backends/multi/libmulti.la matcher/libmatcher.la stemmer/libstem.la
