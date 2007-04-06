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
	bin/xapian-check\
	bin/xapian-compact

dist_man_MANS +=\
	bin/xapian-check.1\
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
	bin/xapian-check\
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

bin_xapian_check_CXXFLAGS = -I$(top_srcdir)/backends/flint
bin_xapian_check_SOURCES = bin/xapian-check.cc
bin_xapian_check_LDADD = $(ldflags) libflintcheck.la libxapian.la

bin_xapian_compact_CXXFLAGS = -I$(top_srcdir)/backends/flint
bin_xapian_compact_SOURCES = bin/xapian-compact.cc
bin_xapian_compact_LDADD = $(ldflags) libgetopt.la libxapian.la

bin_xapian_progsrv_SOURCES = bin/xapian-progsrv.cc
bin_xapian_progsrv_LDADD = $(ldflags) libgetopt.la libxapian.la

bin_xapian_tcpsrv_SOURCES = bin/xapian-tcpsrv.cc
bin_xapian_tcpsrv_LDADD = $(ldflags) libgetopt.la libxapian.la

if MAINTAINER_MODE
bin/quartzcheck.1: bin/quartzcheck$(EXEEXT) makemanpage
	./makemanpage bin/quartzcheck $(srcdir)/bin/quartzcheck.cc bin/quartzcheck.1

bin/quartzcompact.1: bin/quartzcompact$(EXEEXT) makemanpage
	./makemanpage bin/quartzcompact $(srcdir)/bin/quartzcompact.cc bin/quartzcompact.1

bin/quartzdump.1: bin/quartzdump$(EXEEXT) makemanpage
	./makemanpage bin/quartzdump $(srcdir)/bin/quartzdump.cc bin/quartzdump.1

bin/xapian-check.1: bin/xapian-check$(EXEEXT) makemanpage
	./makemanpage bin/xapian-check $(srcdir)/bin/xapian-check.cc bin/xapian-check.1

bin/xapian-compact.1: bin/xapian-compact$(EXEEXT) makemanpage
	./makemanpage bin/xapian-compact $(srcdir)/bin/xapian-compact.cc bin/xapian-compact.1

bin/xapian-progsrv.1: bin/xapian-progsrv$(EXEEXT) makemanpage
	./makemanpage bin/xapian-progsrv $(srcdir)/bin/xapian-progsrv.cc bin/xapian-progsrv.1

bin/xapian-tcpsrv.1: bin/xapian-tcpsrv$(EXEEXT) makemanpage
	./makemanpage bin/xapian-tcpsrv $(srcdir)/bin/xapian-tcpsrv.cc bin/xapian-tcpsrv.1
endif
