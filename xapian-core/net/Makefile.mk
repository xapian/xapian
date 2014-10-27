noinst_HEADERS +=\
	net/length.h\
	net/progclient.h\
	net/remoteconnection.h\
	net/remoteserver.h\
	net/remotetcpclient.h\
	net/remotetcpserver.h\
	net/replicatetcpclient.h\
	net/replicatetcpserver.h\
	net/serialise.h\
	net/tcpclient.h\
	net/tcpserver.h

EXTRA_DIST +=\
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
