## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

#noinst_PROGRAMS = nettest omnetclient

noinst_LTLIBRARIES = libnet.la

libnet_la_SOURCES = progclient.cc

#nettest_SOURCES = nettest.cc
#nettest_LDADD = ../libomus.la

#omnetclient_SOURCES = omnetclient.cc
#omnetclient_LDADD = ../libomus.la
