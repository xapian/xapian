## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite

## Test programs to run
TESTS = nettest

check_PROGRAMS = nettest

noinst_PROGRAMS = omprogsrv omtcpsrv

nettest_SOURCES = nettest.cc
nettest_LDADD = ../testsuite/libtest.la ../libomus.la

omprogsrv_SOURCES = omprogsrv.cc
omprogsrv_LDADD = ../testsuite/libtest.la ../libomus.la

omtcpsrv_SOURCES = omtcpsrv.cc
omtcpsrv_LDADD = ../libomus.la

distclean-local: clean-local

clean-local:
	@if [ -d .sleepycat ]; then \
	  echo "removing .sleepycat/"; \
	  $(RM) -r .sleepycat/* ; rmdir .sleepycat ; \
	fi

