EXTRA_DIST +=\
	backends/remote/Makefile

if BUILD_BACKEND_REMOTE
noinst_HEADERS +=\
	backends/remote/remote-database.h\
	backends/remote/remote-document.h\
	backends/remote/net_postlist.h\
	backends/remote/net_termlist.h

lib_src +=\
	backends/remote/remote-document.cc\
	backends/remote/net_postlist.cc\
	backends/remote/net_termlist.cc\
	backends/remote/remote-database.cc
endif
