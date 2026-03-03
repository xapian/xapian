noinst_HEADERS +=\
	api/documenttermlist.h\
	api/documentvaluelist.h\
	api/editdistance.h\
	api/enquireinternal.h\
	api/msetinternal.h\
	api/result.h\
	api/postingiteratorinternal.h\
	api/queryinternal.h\
	api/queryvector.h\
	api/replication.h\
	api/roundestimate.h\
	api/rsetinternal.h\
	api/smallvector.h\
	api/terminfo.h\
	api/termlist.h\
	api/vectortermlist.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/compactor.cc\
	api/constinfo.cc\
	api/database.cc\
	api/decvalwtsource.cc\
	api/document.cc\
	api/documenttermlist.cc\
	api/documentvaluelist.cc\
	api/editdistance.cc\
	api/enquire.cc\
	api/error.cc\
	api/expanddecider.cc\
	api/keymaker.cc\
	api/matchspy.cc\
	api/mset.cc\
	api/msetiterator.cc\
	api/result.cc\
	api/positioniterator.cc\
	api/postingiterator.cc\
	api/postingsource.cc\
	api/query.cc\
	api/queryinternal.cc\
	api/registry.cc\
	api/rset.cc\
	api/smallvector.cc\
	api/sortable-serialise.cc\
	api/terminfo.cc\
	api/termiterator.cc\
	api/termlist.cc\
	api/valueiterator.cc\
	api/valuerangeproc.cc\
	api/valuesetmatchdecider.cc\
	api/vectortermlist.cc

if BUILD_BACKEND_REMOTE
lib_src +=\
	api/replication.cc
endif
