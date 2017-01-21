noinst_HEADERS +=\
	matcher/andmaybepostlist.h\
	matcher/andnotpostlist.h\
	matcher/branchpostlist.h\
	matcher/collapser.h\
	matcher/exactphrasepostlist.h\
	matcher/externalpostlist.h\
	matcher/extraweightpostlist.h\
	matcher/localsubmatch.h\
	matcher/maxpostlist.h\
	matcher/mergepostlist.h\
	matcher/msetcmp.h\
	matcher/msetpostlist.h\
	matcher/multiandpostlist.h\
	matcher/multimatch.h\
	matcher/multixorpostlist.h\
	matcher/nearpostlist.h\
	matcher/orpositionlist.h\
	matcher/orpospostlist.h\
	matcher/orpostlist.h\
	matcher/phrasepostlist.h\
	matcher/queryoptimiser.h\
	matcher/remotesubmatch.h\
	matcher/selectpostlist.h\
	matcher/synonympostlist.h\
	matcher/valuegepostlist.h\
	matcher/valuerangepostlist.h\
	matcher/valuestreamdocument.h

EXTRA_DIST +=\
	matcher/Makefile

if BUILD_BACKEND_REMOTE
lib_src +=\
	matcher/remotesubmatch.cc
endif
# Make sure we always distribute this source.
EXTRA_DIST +=\
	matcher/remotesubmatch.cc

lib_src +=\
	matcher/andmaybepostlist.cc\
	matcher/andnotpostlist.cc\
	matcher/branchpostlist.cc\
	matcher/collapser.cc\
	matcher/exactphrasepostlist.cc\
	matcher/externalpostlist.cc\
	matcher/localsubmatch.cc\
	matcher/maxpostlist.cc\
	matcher/mergepostlist.cc\
	matcher/msetcmp.cc\
	matcher/msetpostlist.cc\
	matcher/multiandpostlist.cc\
	matcher/multimatch.cc\
	matcher/multixorpostlist.cc\
	matcher/nearpostlist.cc\
	matcher/orpositionlist.cc\
	matcher/orpospostlist.cc\
	matcher/orpostlist.cc\
	matcher/phrasepostlist.cc\
	matcher/selectpostlist.cc\
	matcher/synonympostlist.cc\
	matcher/valuegepostlist.cc\
	matcher/valuerangepostlist.cc\
	matcher/valuestreamdocument.cc
