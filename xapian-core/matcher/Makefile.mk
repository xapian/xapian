noinst_HEADERS +=\
	matcher/andmaybepostlist.h\
	matcher/andnotpostlist.h\
	matcher/andpostlist.h\
	matcher/branchpostlist.h\
	matcher/branchtermlist.h\
	matcher/emptysubmatch.h\
	matcher/exactphrasepostlist.h\
	matcher/extraweightpostlist.h\
	matcher/filterpostlist.h\
	matcher/localmatch.h\
	matcher/mergepostlist.h\
	matcher/msetcmp.h\
	matcher/msetpostlist.h\
	matcher/orpostlist.h\
	matcher/ortermlist.h\
	matcher/phrasepostlist.h\
	matcher/remotesubmatch.h\
	matcher/selectpostlist.h\
	matcher/valuerangepostlist.h\
	matcher/xorpostlist.h

EXTRA_DIST +=\
	matcher/dir_contents\
	matcher/Makefile

if BUILD_BACKEND_REMOTE
libxapian_la_SOURCES +=\
	matcher/remotesubmatch.cc
endif
# Make sure we always distribute this source.
EXTRA_DIST +=\
	matcher/remotesubmatch.cc

libxapian_la_SOURCES +=\
	matcher/andmaybepostlist.cc\
	matcher/andnotpostlist.cc\
	matcher/andpostlist.cc\
	matcher/bm25weight.cc\
	matcher/emptysubmatch.cc\
	matcher/exactphrasepostlist.cc\
	matcher/expand.cc\
	matcher/expandweight.cc\
	matcher/filterpostlist.cc\
	matcher/localmatch.cc\
	matcher/mergepostlist.cc\
	matcher/msetcmp.cc\
	matcher/msetpostlist.cc\
	matcher/multimatch.cc\
	matcher/orpostlist.cc\
	matcher/ortermlist.cc\
	matcher/phrasepostlist.cc\
	matcher/rset.cc\
	matcher/selectpostlist.cc\
	matcher/stats.cc\
	matcher/tradweight.cc\
	matcher/valuerangepostlist.cc\
	matcher/weight.cc\
	matcher/xorpostlist.cc
