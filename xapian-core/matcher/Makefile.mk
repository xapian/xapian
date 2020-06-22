noinst_HEADERS +=\
	matcher/andmaybepostlist.h\
	matcher/andnotpostlist.h\
	matcher/boolorpostlist.h\
	matcher/collapser.h\
	matcher/deciderpostlist.h\
	matcher/exactphrasepostlist.h\
	matcher/externalpostlist.h\
	matcher/extraweightpostlist.h\
	matcher/localsubmatch.h\
	matcher/matcher.h\
	matcher/matchtimeout.h\
	matcher/maxpostlist.h\
	matcher/msetcmp.h\
	matcher/multiandpostlist.h\
	matcher/multixorpostlist.h\
	matcher/nearpostlist.h\
	matcher/orpositionlist.h\
	matcher/orpospostlist.h\
	matcher/orpostlist.h\
	matcher/phrasepostlist.h\
	matcher/postlisttree.h\
	matcher/protomset.h\
	matcher/queryoptimiser.h\
	matcher/remotesubmatch.h\
	matcher/selectpostlist.h\
	matcher/spymaster.h\
	matcher/synonympostlist.h\
	matcher/valuegepostlist.h\
	matcher/valuerangepostlist.h\
	matcher/valuestreamdocument.h\
	matcher/wrapperpostlist.h

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
	matcher/boolorpostlist.cc\
	matcher/collapser.cc\
	matcher/deciderpostlist.cc\
	matcher/exactphrasepostlist.cc\
	matcher/externalpostlist.cc\
	matcher/extraweightpostlist.cc\
	matcher/localsubmatch.cc\
	matcher/matcher.cc\
	matcher/maxpostlist.cc\
	matcher/msetcmp.cc\
	matcher/multiandpostlist.cc\
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
	matcher/valuestreamdocument.cc\
	matcher/wrapperpostlist.cc
