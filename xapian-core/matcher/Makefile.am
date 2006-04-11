## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

noinst_HEADERS = orpostlist.h andpostlist.h andnotpostlist.h \
 andmaybepostlist.h xorpostlist.h branchpostlist.h filterpostlist.h \
 phrasepostlist.h branchtermlist.h ortermlist.h selectpostlist.h \
 mergepostlist.h msetpostlist.h extraweightpostlist.h networkmatch.h \
 localmatch.h emptymatch.h biaspostlist.h msetcmp.h

noinst_LTLIBRARIES = libmatcher.la

if BUILD_BACKEND_REMOTE
SOURCEFORREMOTE = networkmatch.cc
endif
# ship remote specific sources even if this tree isn't configured for them
EXTRA_DIST = networkmatch.cc

libmatcher_la_SOURCES = orpostlist.cc andpostlist.cc andnotpostlist.cc \
 andmaybepostlist.cc xorpostlist.cc phrasepostlist.cc selectpostlist.cc \
 filterpostlist.cc ortermlist.cc expandweight.cc rset.cc \
 bm25weight.cc tradweight.cc \
 localmatch.cc multimatch.cc expand.cc stats.cc \
 mergepostlist.cc msetpostlist.cc msetcmp.cc \
 $(SOURCEFORREMOTE)
