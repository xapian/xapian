## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_PROGRAMS = nettest omprogsrv

nettest_SOURCES = nettest.cc
nettest_LDADD = ../libomus.la

omprogsrv_SOURCES = omprogsrv.cc
omprogsrv_LDADD = ../libomus.la
