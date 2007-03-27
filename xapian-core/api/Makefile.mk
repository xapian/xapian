noinst_HEADERS +=\
	api/maptermlist.h

EXTRA_DIST +=\
	api/dir_contents\
	api/Makefile

libxapian_la_SOURCES +=\
	api/errorhandler.cc\
	api/omdatabase.cc\
	api/omdocument.cc\
	api/omenquire.cc\
	api/ompositionlistiterator.cc\
	api/ompostlistiterator.cc\
	api/omquery.cc\
	api/omqueryinternal.cc\
	api/omtermlistiterator.cc\
	api/omvalueiterator.cc\
	api/valuerangeproc.cc\
	api/version.cc
