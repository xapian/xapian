## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

if BUILD_BACKEND_QUARTZ
quartz_bin_programs = quartzcheck quartzcompact quartzdump
quartz_mans = quartzcheck.1 quartzcompact.1 quartzdump.1
endif

if BUILD_BACKEND_FLINT
flint_bin_programs = xapian-compact
flint_mans = xapian-compact.1
endif

if BUILD_BACKEND_REMOTE
remote_bin_programs = xapian-tcpsrv xapian-progsrv
remote_mans = xapian-tcpsrv.1
endif

EXTRA_PROGRAMS = quartzcheck quartzcompact quartzdump\
    xapian-compact xapian-tcpsrv xapian-progsrv
bin_PROGRAMS = $(quartz_bin_programs) $(remote_bin_programs) $(flint_bin_programs)

# testsuite needed for btreecheck.h
quartzcheck_CXXFLAGS = -I$(top_srcdir)/backends/quartz -I$(top_srcdir)/testsuite
quartzcheck_SOURCES = quartzcheck.cc
quartzcheck_LDADD = @ldflags@ ../testsuite/libbtreecheck.la ../libxapian.la

quartzcompact_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzcompact_SOURCES = quartzcompact.cc
quartzcompact_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

quartzdump_CXXFLAGS = -I$(top_srcdir)/backends/quartz
quartzdump_SOURCES = quartzdump.cc
quartzdump_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

xapian_progsrv_SOURCES = xapian-progsrv.cc
xapian_progsrv_LDADD = @ldflags@ ../libxapian.la

xapian_tcpsrv_SOURCES = xapian-tcpsrv.cc
xapian_tcpsrv_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

xapian_compact_CXXFLAGS = -I$(top_srcdir)/backends/flint
xapian_compact_SOURCES = xapian-compact.cc
xapian_compact_LDADD = @ldflags@ ../getopt/libgetopt.la ../libxapian.la

dist_man_MANS = $(quartz_mans) $(flint_mans) $(remote_mans)

MAINTAINERCLEANFILES = $(dist_man_MANS)

if MAINTAINER_MODE
CLEANFILES = $(dist_man_MANS)

quartzcheck.1: quartzcheck
	$(HELP2MAN) -o quartzcheck.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/quartzcheck.cc`" ./quartzcheck

quartzcompact.1: quartzcompact
	$(HELP2MAN) -o quartzcompact.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/quartzcompact.cc`" ./quartzcompact

quartzdump.1: quartzdump
	$(HELP2MAN) -o quartzdump.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/quartzdump.cc`" ./quartzdump

xapian-tcpsrv.1: xapian-tcpsrv
	$(HELP2MAN) -o xapian-tcpsrv.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/xapian-tcpsrv.cc`" ./xapian-tcpsrv

xapian-compact.1: xapian-compact
	$(HELP2MAN) -o xapian-compact.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/xapian-compact.cc`" ./xapian-compact
endif
