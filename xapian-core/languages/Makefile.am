## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = pool.h

noinst_LTLIBRARIES = liblanguages.la

liblanguages_la_SOURCES = pool.c

SUBDIRS = dutch english french german italian portuguese spanish swedish \
          porter .

liblanguages_la_LIBADD = \
dutch/libdutch.la \
english/libenglish.la \
french/libfrench.la \
german/libgerman.la \
italian/libitalian.la \
portuguese/libportuguese.la \
spanish/libspanish.la \
swedish/libswedish.la \
porter/libporter.la \
