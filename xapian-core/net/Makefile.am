## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = readquery.h

noinst_LTLIBRARIES = libnet.la

libnet_la_SOURCES = socketclient.cc progclient.cc progserver.cc \
		    socketcommon.cc tcpclient.cc readquery.cc \
		    socketserver.cc tcpserver.cc omerr_string.cc \
		    omlinebuf.cc
BUILT_SOURCES = readquery.cc
EXTRA_DIST = readquery.ll

readquery.cc: $(srcdir)/readquery.ll
	$(LEX) -oreadquery.tmp $(AM_LFLAGS) $(srcdir)/readquery.ll $(LFLAGS)
	: # config.h must be included before any other headers, but that
	: # seems to be impossible to arrange in lex/flex
	echo "#include <config.h>" | cat - readquery.tmp > readquery.tmp2
	rm readquery.tmp
	mv readquery.tmp2 readquery.cc
