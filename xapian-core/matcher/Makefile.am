## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = orpostlist.h andpostlist.h andnotpostlist.h \
 andmaybepostlist.h xorpostlist.h branchpostlist.h filterpostlist.h \
 phrasepostlist.h branchtermlist.h ortermlist.h \
 selectpostlist.h

noinst_LTLIBRARIES = libmatcher.la

if BUILD_BACKEND_NET
SOURCEFORNET = networkmatch.cc
endif

libmatcher_la_SOURCES = orpostlist.cc andpostlist.cc \
 andnotpostlist.cc andmaybepostlist.cc xorpostlist.cc phrasepostlist.cc \
 irweight.cc boolweight.h bm25weight.h bm25weight.cc tradweight.h \
 tradweight.cc \
 filterpostlist.cc ortermlist.cc expandweight.cc rset.cc \
 localmatch.cc multimatch.cc expand.cc stats.cc \
 leafdocument.cc selectpostlist.cc \
 $(SOURCEFORNET)
