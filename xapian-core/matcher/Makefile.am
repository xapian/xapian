## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = orpostlist.h andpostlist.h andnotpostlist.h \
 andmaybepostlist.h xorpostlist.h branchpostlist.h filterpostlist.h \
 emptypostlist.h branchtermlist.h ortermlist.h

noinst_LTLIBRARIES = libmatcher.la

libmatcher_la_SOURCES = orpostlist.cc andpostlist.cc \
 andnotpostlist.cc andmaybepostlist.cc xorpostlist.cc \
 bm25weight.h bm25weight.cc tradweight.h tradweight.cc \
 ortermlist.cc expandweight.cc rset.cc \
 match.cc expand.cc
