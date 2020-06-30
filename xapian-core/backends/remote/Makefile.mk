EXTRA_DIST +=\
	backends/remote/Makefile

if BUILD_BACKEND_REMOTE
noinst_HEADERS +=\
	backends/remote/net_postlist.h\
	backends/remote/remote_alltermslist.h\
	backends/remote/remote-database.h\
	backends/remote/remote-document.h\
	backends/remote/remote_keylist.h\
	backends/remote/remote_termlist.h

lib_src +=\
	backends/remote/net_postlist.cc\
	backends/remote/remote_alltermslist.cc\
	backends/remote/remote-database.cc\
	backends/remote/remote-document.cc\
	backends/remote/remote_keylist.cc\
	backends/remote/remote_termlist.cc

endif
