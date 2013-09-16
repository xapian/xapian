noinst_HEADERS +=\
	unicode/description_append.h

EXTRA_DIST +=\
	unicode/dir_contents\
	unicode/Makefile

lib_src +=\
	unicode/description_append.cc\
	unicode/tclUniData.cc\
	unicode/utf8itor.cc
