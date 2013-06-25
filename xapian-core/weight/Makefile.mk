noinst_HEADERS +=\
	weight/weightinternal.h

EXTRA_DIST +=\
	weight/dir_contents\
	weight/Makefile

lib_src +=\
	weight/bm25weight.cc\
	weight/boolweight.cc\
	weight/ineb2weight.cc\
	weight/tradweight.cc\
	weight/tfidfweight.cc\
	weight/weight.cc\
	weight/weightinternal.cc
