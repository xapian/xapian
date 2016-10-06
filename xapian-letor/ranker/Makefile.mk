noinst_HEADERS +=\
	common/serialise-double.h

EXTRA_DIST +=\
	ranker/Makefile

lib_src +=\
	common/serialise-double.cc\
	ranker/listnet_ranker.cc\
	ranker/ranker.cc
