noinst_HEADERS +=\
	feature/feature_internal.h

EXTRA_DIST +=\
	feature/Makefile

lib_src +=\
	feature/feature.cc\
	feature/feature_internal.cc\
	feature/tffeature.cc\
	feature/tfdoclenfeature.cc\
	feature/idffeature.cc\
	feature/colltfcolllenfeature.cc\
	feature/tfidfdoclenfeature.cc\
	feature/tfdoclencolltfcolllenfeature.cc
