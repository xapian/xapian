## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${srcdir}/..

noinst_LTLIBRARIES = libportuguese.la

noinst_HEADERS = stem_portuguese.h stemmer_portuguese.h

libportuguese_la_SOURCES = stem_portuguese.c
