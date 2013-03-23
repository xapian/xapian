noinst_HEADERS +=\
	weight/weightinternal.h

EXTRA_DIST +=\
	weight/dir_contents\
	weight/Makefile

lib_src +=\
	weight/bm25weight.cc\
	weight/boolweight.cc\
	weight/tradweight.cc\
	weight/weight.cc\
	weight/dfr_pl2weight.cc\
	weight/weightinternal.cc
