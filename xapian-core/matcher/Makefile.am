## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

include_HEADERS = orpostlist.h andpostlist.h andnotpostlist.h \
 andmaybepostlist.h xorpostlist.h branchpostlist.h filterpostlist.h

noinst_LTLIBRARIES = libmatcher.la

libmatcher_la_SOURCES = match.cc orpostlist.cc andpostlist.cc \
 andnotpostlist.cc andmaybepostlist.cc xorpostlist.cc irweight.cc
