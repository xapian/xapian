noinst_HEADERS +=\
	common/serialise-double.h

EXTRA_DIST +=\
	ranker/Makefile

lib_src +=\
	ranker/listnet_ranker.cc\
	ranker/ranker.cc\
	common/serialise-double.cc\
	ranker/svmranker.cc
