## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = readquery.h

noinst_LTLIBRARIES = libnet.la

libnet_la_SOURCES = progclient.cc progserver.cc progcommon.cc readquery.ll
