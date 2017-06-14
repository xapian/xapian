noinst_HEADERS +=\
	api/featurelist_internal.h\
	api/featurevector_internal.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/featurelist.cc\
	api/featurelist_internal.cc\
	api/featurevector.cc
