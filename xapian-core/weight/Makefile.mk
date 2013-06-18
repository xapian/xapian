noinst_HEADERS +=\
	weight/weightinternal.h

EXTRA_DIST +=\
	weight/dir_contents\
	weight/Makefile

lib_src +=\
       weight/bb2weight.cc\
	weight/bm25weight.cc\
	weight/boolweight.cc\
	weight/tradweight.cc\
	weight/tfidfweight.cc\
	weight/weight.cc\
	weight/weightinternal.cc
