noinst_HEADERS +=\
	matcher/andmaybepostlist.h\
	matcher/andnotpostlist.h\
	matcher/branchpostlist.h\
	matcher/exactphrasepostlist.h\
	matcher/extraweightpostlist.h\
	matcher/localmatch.h\
	matcher/mergepostlist.h\
	matcher/msetcmp.h\
	matcher/msetpostlist.h\
	matcher/multiandpostlist.h\
	matcher/multixorpostlist.h\
	matcher/orpostlist.h\
	matcher/phrasepostlist.h\
	matcher/queryoptimiser.h\
	matcher/remotesubmatch.h\
	matcher/scaleweight.h\
	matcher/selectpostlist.h\
	matcher/valuegepostlist.h\
	matcher/valuerangepostlist.h

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
	matcher/bm25weight.cc\
	matcher/branchpostlist.cc\
	matcher/exactphrasepostlist.cc\
	matcher/localmatch.cc\
	matcher/mergepostlist.cc\
	matcher/msetcmp.cc\
	matcher/msetpostlist.cc\
	matcher/multiandpostlist.cc\
	matcher/multimatch.cc\
	matcher/multixorpostlist.cc\
	matcher/orpostlist.cc\
	matcher/phrasepostlist.cc\
	matcher/queryoptimiser.cc\
	matcher/rset.cc\
	matcher/scaleweight.cc\
	matcher/selectpostlist.cc\
	matcher/stats.cc\
	matcher/tradweight.cc\
	matcher/valuegepostlist.cc\
	matcher/valuerangepostlist.cc\
	matcher/weight.cc
