## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

## Test programs to run
TESTS = nettest

check_PROGRAMS = nettest

noinst_PROGRAMS = omprogsrv omtcpsrv

nettest_SOURCES = nettest.cc
nettest_LDADD = ../libomus.la ../testsuite/libtest.la

omprogsrv_SOURCES = omprogsrv.cc
omprogsrv_LDADD = ../libomus.la

omtcpsrv_SOURCES = omtcpsrv.cc
omtcpsrv_LDADD = ../libomus.la
