EXTRA_DIST +=\
	backends/remote/dir_contents\
	backends/remote/Makefile

if BUILD_BACKEND_REMOTE
noinst_HEADERS +=\
	backends/remote/net_document.h\
	backends/remote/net_postlist.h\
	backends/remote/net_termlist.h

libxapian_la_SOURCES +=\
	backends/remote/net_document.cc\
	backends/remote/net_postlist.cc\
	backends/remote/net_termlist.cc\
	backends/remote/remote-database.cc
endif
