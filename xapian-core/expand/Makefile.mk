noinst_HEADERS +=\
	expand/esetinternal.h\
	expand/expandweight.h\
	expand/ortermlist.h

EXTRA_DIST +=\
	expand/dir_contents\
	expand/Makefile

lib_src +=\
	expand/esetinternal.cc\
	expand/expandweight.cc\
	expand/ortermlist.cc
