EXTRA_DIST +=\
	backends/multi/dir_contents\
	backends/multi/Makefile

noinst_HEADERS +=\
	backends/multi/multi_postlist.h\
	backends/multi/multi_termlist.h

libxapian_la_SOURCES +=\
	backends/multi/multi_alltermslist.cc\
	backends/multi/multi_postlist.cc\
	backends/multi/multi_termlist.cc
