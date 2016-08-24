noinst_HEADERS +=\
	api/featurevector_internal.h\
	api/letor_internal.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/featurelist.cc\
	api/featurevector.cc\
	api/letor.cc\
	api/letor_internal.cc