## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${srcdir}/..

noinst_LTLIBRARIES = libspanish.la

noinst_HEADERS = stem_spanish.h stemmer_spanish.h

libspanish_la_SOURCES = stem_spanish.c
