if BUILD_BACKEND_QUARTZ
bin_PROGRAMS +=\
	bin/quartzcheck\
	bin/quartzcompact\
	bin/quartzdump

dist_man_MANS +=\
	bin/quartzcheck.1\
	bin/quartzcompact.1\
	bin/quartzdump.1
endif

if BUILD_BACKEND_FLINT
bin_PROGRAMS +=\
	bin/xapian-compact

dist_man_MANS +=\
	bin/xapian-compact.1
endif

if BUILD_BACKEND_REMOTE
bin_PROGRAMS +=\
	bin/xapian-progsrv\
	bin/xapian-tcpsrv

dist_man_MANS +=\
	bin/xapian-progsrv.1\
	bin/xapian-tcpsrv.1
endif

EXTRA_PROGRAMS +=\
	bin/quartzcheck\
	bin/quartzcompact\
	bin/quartzdump\
	bin/xapian-compact\
	bin/xapian-progsrv\
	bin/xapian-tcpsrv

bin_quartzcheck_CXXFLAGS = -I$(top_srcdir)/backends/quartz
bin_quartzcheck_SOURCES = bin/quartzcheck.cc
bin_quartzcheck_LDADD = $(ldflags) libquartzcheck.la libxapian.la

bin_quartzcompact_CXXFLAGS = -I$(top_srcdir)/backends/quartz
bin_quartzcompact_SOURCES = bin/quartzcompact.cc
bin_quartzcompact_LDADD = $(ldflags) libgetopt.la libxapian.la

bin_quartzdump_CXXFLAGS = -I$(top_srcdir)/backends/quartz
bin_quartzdump_SOURCES = bin/quartzdump.cc
bin_quartzdump_LDADD = $(ldflags) libgetopt.la libxapian.la

bin_xapian_compact_CXXFLAGS = -I$(top_srcdir)/backends/flint
bin_xapian_compact_SOURCES = bin/xapian-compact.cc
bin_xapian_compact_LDADD = $(ldflags) libgetopt.la libxapian.la

bin_xapian_progsrv_SOURCES = bin/xapian-progsrv.cc
bin_xapian_progsrv_LDADD = $(ldflags) libgetopt.la libxapian.la

bin_xapian_tcpsrv_SOURCES = bin/xapian-tcpsrv.cc
bin_xapian_tcpsrv_LDADD = $(ldflags) libgetopt.la libxapian.la

if MAINTAINER_MODE
bin/quartzcheck.1: bin/quartzcheck
	$(HELP2MAN) -o bin/quartzcheck.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/bin/quartzcheck.cc`" bin/quartzcheck

bin/quartzcompact.1: bin/quartzcompact
	$(HELP2MAN) -o bin/quartzcompact.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/bin/quartzcompact.cc`" bin/quartzcompact

bin/quartzdump.1: bin/quartzdump
	$(HELP2MAN) -o bin/quartzdump.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/bin/quartzdump.cc`" bin/quartzdump

bin/xapian-bin/progsrv.1: bin/xapian-bin/progsrv
	$(HELP2MAN) -o bin/xapian-progsrv.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/bin/xapian-progsrv.cc`" bin/xapian-progsrv

bin/xapian-bin/tcpsrv.1: bin/xapian-bin/tcpsrv
	$(HELP2MAN) -o bin/xapian-tcpsrv.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/bin/xapian-tcpsrv.cc`" bin/xapian-tcpsrv

bin/xapian-bin/compact.1: bin/xapian-bin/compact
	$(HELP2MAN) -o bin/xapian-compact.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/bin/xapian-compact.cc`" bin/xapian-compact
endif
