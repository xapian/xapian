noinst_HEADERS +=\
	net/length.h\
	net/progclient.h\
	net/remoteconnection.h\
	net/remoteserver.h\
	net/remotetcpclient.h\
	net/remotetcpserver.h\
	net/replicatetcpclient.h\
	net/replicatetcpserver.h\
	net/resolver.h\
	net/serialise.h\
	net/serialise-error.h\
	net/tcpclient.h\
	net/tcpserver.h

EXTRA_DIST +=\
	net/remote_protocol.rst\
	net/replication_protocol.rst\
	net/Makefile

lib_src +=\
	net/length.cc\
	net/serialise.cc

if BUILD_BACKEND_REMOTE
lib_src +=\
	net/progclient.cc\
	net/remoteconnection.cc\
	net/remoteserver.cc\
	net/remotetcpclient.cc\
	net/remotetcpserver.cc\
	net/replicatetcpclient.cc\
	net/replicatetcpserver.cc\
	net/serialise-error.cc\
	net/tcpclient.cc\
	net/tcpserver.cc
endif
