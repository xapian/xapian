## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS =

noinst_LTLIBRARIES = liblanguages.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
liblanguages_la_SOURCES = dummy.c

SUBDIRS = en .

liblanguages_la_LIBADD = en/liben.la
