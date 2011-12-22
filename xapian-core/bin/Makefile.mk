EXTRA_DIST +=\
	bin/dir_contents\
	bin/Makefile

bin_PROGRAMS +=\
	bin/xapian-delve

if BUILD_BACKEND_BRASS_OR_CHERT
bin_PROGRAMS +=\
	bin/xapian-check\
	bin/xapian-compact\
	bin/xapian-inspect\
	bin/xapian-replicate\
	bin/xapian-replicate-server

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/xapian-check.1\
	bin/xapian-compact.1\
	bin/xapian-delve.1\
	bin/xapian-inspect.1\
	bin/xapian-replicate.1\
	bin/xapian-replicate-server.1
endif
endif

if BUILD_BACKEND_REMOTE
bin_PROGRAMS +=\
	bin/xapian-progsrv\
	bin/xapian-tcpsrv

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/xapian-progsrv.1\
	bin/xapian-tcpsrv.1
endif
endif

bin_xapian_check_CPPFLAGS =\
	-I$(top_srcdir)/backends/brass\
	-I$(top_srcdir)/backends/chert
bin_xapian_check_SOURCES = bin/xapian-check.cc
bin_xapian_check_LDADD = $(ldflags)
if BUILD_BACKEND_BRASS
bin_xapian_check_SOURCES += bin/xapian-check-brass.cc bin/xapian-check-brass.h
bin_xapian_check_LDADD += libbrasscheck.la
endif
if BUILD_BACKEND_CHERT
bin_xapian_check_SOURCES += bin/xapian-check-chert.cc bin/xapian-check-chert.h
bin_xapian_check_LDADD += libchertcheck.la
endif
bin_xapian_check_LDADD += $(libxapian_la)

bin_xapian_compact_SOURCES = bin/xapian-compact.cc
bin_xapian_compact_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_delve_SOURCES = bin/xapian-delve.cc
bin_xapian_delve_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_inspect_CPPFLAGS = -I$(top_srcdir)/backends/chert
bin_xapian_inspect_SOURCES = bin/xapian-inspect.cc
bin_xapian_inspect_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_progsrv_SOURCES = bin/xapian-progsrv.cc
bin_xapian_progsrv_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_replicate_SOURCES = bin/xapian-replicate.cc
bin_xapian_replicate_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_replicate_server_SOURCES = bin/xapian-replicate-server.cc
bin_xapian_replicate_server_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_tcpsrv_SOURCES = bin/xapian-tcpsrv.cc
bin_xapian_tcpsrv_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

if DOCUMENTATION_RULES
bin/xapian-check.1: bin/xapian-check$(EXEEXT) makemanpage
	./makemanpage bin/xapian-check $(srcdir)/bin/xapian-check.cc bin/xapian-check.1

bin/xapian-compact.1: bin/xapian-compact$(EXEEXT) makemanpage
	./makemanpage bin/xapian-compact $(srcdir)/bin/xapian-compact.cc bin/xapian-compact.1

bin/xapian-delve.1: bin/xapian-delve$(EXEEXT) makemanpage
	./makemanpage bin/xapian-delve $(srcdir)/bin/xapian-delve.cc bin/xapian-delve.1

bin/xapian-inspect.1: bin/xapian-inspect$(EXEEXT) makemanpage
	./makemanpage bin/xapian-inspect $(srcdir)/bin/xapian-inspect.cc bin/xapian-inspect.1

bin/xapian-progsrv.1: bin/xapian-progsrv$(EXEEXT) makemanpage
	./makemanpage bin/xapian-progsrv $(srcdir)/bin/xapian-progsrv.cc bin/xapian-progsrv.1

bin/xapian-replicate.1: bin/xapian-replicate$(EXEEXT) makemanpage
	./makemanpage bin/xapian-replicate $(srcdir)/bin/xapian-replicate.cc bin/xapian-replicate.1

bin/xapian-replicate-server.1: bin/xapian-replicate-server$(EXEEXT) makemanpage
	./makemanpage bin/xapian-replicate-server $(srcdir)/bin/xapian-replicate-server.cc bin/xapian-replicate-server.1

bin/xapian-tcpsrv.1: bin/xapian-tcpsrv$(EXEEXT) makemanpage
	./makemanpage bin/xapian-tcpsrv $(srcdir)/bin/xapian-tcpsrv.cc bin/xapian-tcpsrv.1
endif
