noinst_HEADERS +=\
	api/documenttermlist.h\
	api/documentvaluelist.h\
	api/editdistance.h\
	api/emptypostlist.h\
	api/leafpostlist.h\
	api/omenquireinternal.h\
	api/postlist.h\
	api/queryinternal.h\
	api/queryvector.h\
	api/replication.h\
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
	api/decvalwtsource.cc\
	api/documenttermlist.cc\
	api/documentvaluelist.cc\
	api/editdistance.cc\
	api/emptypostlist.cc\
	api/error.cc\
	api/errorhandler.cc\
	api/expanddecider.cc\
	api/keymaker.cc\
	api/leafpostlist.cc\
	api/matchspy.cc\
	api/omdatabase.cc\
	api/omdocument.cc\
	api/omenquire.cc\
	api/positioniterator.cc\
	api/postingiterator.cc\
	api/postingsource.cc\
	api/postlist.cc\
	api/query.cc\
	api/queryinternal.cc\
	api/registry.cc\
	api/replication.cc\
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
