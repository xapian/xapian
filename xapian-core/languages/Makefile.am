## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = pool.h

noinst_LTLIBRARIES = liblanguages.la

liblanguages_la_SOURCES = pool.c stemmer_builder.cc

SUBDIRS = dutch english french german italian portuguese spanish .

liblanguages_la_LIBADD = \
dutch/libdutch.la \
english/libenglish.la \
french/libfrench.la \
german/libgerman.la \
italian/libitalian.la \
portuguese/libportuguese.la \
spanish/libspanish.la \
