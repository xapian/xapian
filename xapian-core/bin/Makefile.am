## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

if BUILD_BACKEND_QUARTZ
quartz_bin_programs = quartzcheck quartzcompact quartzdump
endif

if BUILD_BACKEND_REMOTE
remote_bin_programs = omprogsrv omtcpsrv
endif

bin_PROGRAMS = $(quartz_bin_programs) $(remote_bin_programs)

quartzcheck_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcheck_SOURCES = quartzcheck.cc $(top_srcdir)/backends/quartz/btreecheck.cc
quartzcheck_LDADD = ../libxapian.la

quartzcompact_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcompact_SOURCES = quartzcompact.cc
quartzcompact_LDADD = ../libxapian.la 

quartzdump_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzdump_SOURCES = quartzdump.cc
quartzdump_LDADD = ../getopt/libgetopt.la ../libxapian.la

omprogsrv_CXXFLAGS = -I$(top_srcdir)/testsuite
omprogsrv_SOURCES = omprogsrv.cc
omprogsrv_LDADD = ../testsuite/libtest.la ../libxapian.la

omtcpsrv_SOURCES = omtcpsrv.cc
omtcpsrv_LDADD = ../getopt/libgetopt.la ../libxapian.la
