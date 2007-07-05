noinst_HEADERS +=\
	api/maptermlist.h\
	api/editdistance.h

EXTRA_DIST +=\
	api/dir_contents\
	api/Makefile

libxapian_la_SOURCES +=\
	api/editdistance.cc\
	api/error.cc\
	api/errorhandler.cc\
	api/expanddecider.cc\
	api/matchspy.cc\
	api/omdatabase.cc\
	api/omdocument.cc\
	api/omenquire.cc\
	api/ompositionlistiterator.cc\
	api/ompostlistiterator.cc\
	api/omquery.cc\
	api/omqueryinternal.cc\
	api/omtermlistiterator.cc\
	api/omvalueiterator.cc\
	api/sortable-serialise.cc\
	api/termlist.cc\
	api/valuerangeproc.cc\
	api/valuerangeproccompat.cc\
	api/version.cc
