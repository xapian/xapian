noinst_HEADERS +=\
	api/featurevector_internal.h\
	api/featurelist_internal.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/featurelist.cc\
	api/featurevector.cc\
	api/featurelist_internal.cc
