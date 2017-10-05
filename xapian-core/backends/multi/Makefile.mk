EXTRA_DIST +=\
	backends/multi/Makefile

noinst_HEADERS +=\
	backends/multi/multi_alltermslist.h\
	backends/multi/multi_database.h\
	backends/multi/multi_postlist.h\
	backends/multi/multi_termlist.h\
	backends/multi/multi_valuelist.h

lib_src +=\
	backends/multi/multi_alltermslist.cc\
	backends/multi/multi_database.cc\
	backends/multi/multi_postlist.cc\
	backends/multi/multi_termlist.cc\
	backends/multi/multi_valuelist.cc
