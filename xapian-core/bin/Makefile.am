## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include -I$(top_srcdir)/testsuite

if BUILD_BACKEND_QUARTZ
quartz_bin_programs = quartzcheck quartzcompact quartzdump
endif

if BUILD_BACKEND_REMOTE
remote_bin_programs = omtcpsrv
remote_check_programs = omprogsrv
endif

EXTRA_PROGRAMS = quartzcheck quartzcompact quartzdump omtcpsrv omprogsrv
bin_PROGRAMS = $(quartz_bin_programs) $(remote_bin_programs)
check_PROGRAMS = $(remote_check_programs)

quartzcheck_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcheck_SOURCES = quartzcheck.cc
quartzcheck_LDADD = @ldflags@ ../testsuite/libbtreecheck.la ../libxapian.la

quartzcompact_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcompact_SOURCES = quartzcompact.cc
quartzcompact_LDADD = @ldflags@ ../libxapian.la 

quartzdump_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzdump_SOURCES = quartzdump.cc
quartzdump_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la -lz

omprogsrv_CXXFLAGS = -I$(top_srcdir)/testsuite
omprogsrv_SOURCES = omprogsrv.cc
omprogsrv_LDADD = @ldflags@ ../testsuite/libtest.la ../libxapian.la

omtcpsrv_SOURCES = omtcpsrv.cc
omtcpsrv_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la
