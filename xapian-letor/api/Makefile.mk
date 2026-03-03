noinst_HEADERS +=\
	api/feature_internal.h\
	api/featurelist_internal.h\
	api/featurevector_internal.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/feature_internal.cc\
	api/featurelist.cc\
	api/featurelist_internal.cc\
	api/featurevector.cc
