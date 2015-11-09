noinst_HEADERS +=\
	net/length.h

EXTRA_DIST +=\
	net/dir_contents\
	net/Makefile

if BUILD_BACKEND_REMOTE
lib_src +=\
	net/length.cc\
	net/progclient.cc\
	net/remoteconnection.cc\
	net/remoteserver.cc\
	net/remotetcpclient.cc\
	net/remotetcpserver.cc\
	net/replicatetcpclient.cc\
	net/replicatetcpserver.cc\
	net/serialise.cc\
	net/tcpclient.cc\
	net/tcpserver.cc
endif
