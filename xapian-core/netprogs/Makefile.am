## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

noinst_PROGRAMS = omprogsrv omtcpsrv

omprogsrv_SOURCES = omprogsrv.cc
omprogsrv_LDADD = ../testsuite/libtest.la ../libxapian.la

omtcpsrv_SOURCES = omtcpsrv.cc
omtcpsrv_LDADD = ../getopt/libgetopt.la ../libxapian.la
