noinst_HEADERS +=\
	api/letor_internal.h\
	api/featuremanager_internal.h\
	api/featurevector_internal.h

EXTRA_DIST +=\
	api/Makefile

lib_src +=\
	api/featuremanager.cc\
	api/featuremanager_internal.cc\
	api/featurevector.cc\
	api/featurevector_internal.cc\
	api/letor.cc\
	api/letor_features.cc\
	api/letor_internal.cc\
	api/ranker.cc\
	api/scorer.cc

