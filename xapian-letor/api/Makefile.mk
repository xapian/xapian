noinst_HEADERS +=\
	api/letor_internal.h\
	api/featurevector_internal.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/featurelist.cc\
	api/featurevector.cc\
	api/letor.cc\
	api/letor_internal.cc\
	api/scorer.cc

