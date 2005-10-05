## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include -I$(top_srcdir)/testsuite

if BUILD_BACKEND_QUARTZ
quartz_bin_programs = quartzcheck quartzcompact quartzdump
endif

if BUILD_BACKEND_FLINT
flint_bin_programs = xapian-compact
endif

if BUILD_BACKEND_REMOTE
remote_bin_programs = xapian-tcpsrv
remote_check_programs = xapian-progsrv
endif

EXTRA_PROGRAMS = quartzcheck quartzcompact quartzdump\
    xapian-compact xapian-tcpsrv xapian-progsrv
bin_PROGRAMS = $(quartz_bin_programs) $(remote_bin_programs) $(flint_bin_programs)
check_PROGRAMS = $(remote_check_programs)

quartzcheck_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcheck_SOURCES = quartzcheck.cc
quartzcheck_LDADD = @ldflags@ ../testsuite/libbtreecheck.la ../libxapian.la

quartzcompact_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcompact_SOURCES = quartzcompact.cc
quartzcompact_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la 

quartzdump_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzdump_SOURCES = quartzdump.cc
quartzdump_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

xapian_progsrv_CXXFLAGS = -I$(top_srcdir)/testsuite
xapian_progsrv_SOURCES = omprogsrv.cc
xapian_progsrv_LDADD = @ldflags@ ../testsuite/libtest.la ../libxapian.la

xapian_tcpsrv_SOURCES = omtcpsrv.cc
xapian_tcpsrv_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

xapian_compact_CXXFLAGS = -I$(top_srcdir)/backends/flint
xapian_compact_SOURCES = xapian-compact.cc
xapian_compact_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la 
