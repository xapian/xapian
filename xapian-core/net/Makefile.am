## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = readquery.h

noinst_LTLIBRARIES = libnet.la

libnet_la_SOURCES = socketclient.cc progclient.cc progserver.cc \
		    socketcommon.cc tcpclient.cc readquery.ll \
		    socketserver.cc tcpserver.cc omerr_string.cc
