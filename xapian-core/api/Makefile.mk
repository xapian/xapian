noinst_HEADERS +=\
	api/documentvaluelist.h\
	api/editdistance.h\
	api/maptermlist.h

EXTRA_DIST +=\
	api/dir_contents\
	api/Makefile

lib_src +=\
	api/decvalwtsource.cc\
	api/documentvaluelist.cc\
	api/editdistance.cc\
	api/emptypostlist.cc\
	api/error.cc\
	api/errorhandler.cc\
	api/expanddecider.cc\
	api/leafpostlist.cc\
	api/matchspy.cc\
	api/omdatabase.cc\
	api/omdocument.cc\
	api/omenquire.cc\
	api/ompositionlistiterator.cc\
	api/ompostlistiterator.cc\
	api/omquery.cc\
	api/omqueryinternal.cc\
	api/omtermlistiterator.cc\
	api/postingsource.cc\
	api/postlist.cc\
	api/replication.cc\
	api/serialisationcontext.cc\
	api/sortable-serialise.cc\
	api/sorter.cc\
	api/termlist.cc\
	api/valueiterator.cc\
	api/valuerangeproc.cc\
	api/valuesetmatchdecider.cc\
	api/version.cc
