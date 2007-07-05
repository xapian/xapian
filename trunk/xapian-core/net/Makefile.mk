EXTRA_DIST +=\
	net/dir_contents\
	net/Makefile

if BUILD_BACKEND_REMOTE
libxapian_la_SOURCES +=\
	net/progclient.cc\
	net/remoteconnection.cc\
	net/remoteserver.cc\
	net/serialise.cc\
	net/tcpclient.cc\
	net/tcpserver.cc
endif
