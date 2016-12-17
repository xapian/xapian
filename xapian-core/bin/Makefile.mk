EXTRA_DIST +=\
	bin/Makefile

bin_PROGRAMS +=\
	bin/xapian-delve

noinst_PROGRAMS =

if BUILD_BACKEND_CHERT_OR_GLASS
bin_PROGRAMS +=\
	bin/xapian-check\
	bin/xapian-compact\
	bin/xapian-replicate\
	bin/xapian-replicate-server

if BUILD_BACKEND_CHERT
noinst_PROGRAMS +=\
	bin/xapian-inspect
endif

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/xapian-check.1\
	bin/xapian-compact.1\
	bin/xapian-delve.1\
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

bin_xapian_check_SOURCES = bin/xapian-check.cc
bin_xapian_check_LDADD = $(ldflags) $(libxapian_la)

bin_xapian_compact_SOURCES = bin/xapian-compact.cc
bin_xapian_compact_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_delve_SOURCES = bin/xapian-delve.cc
bin_xapian_delve_LDADD = $(ldflags) libgetopt.la $(libxapian_la)

bin_xapian_inspect_CPPFLAGS =\
	$(AM_CPPFLAGS)\
	-DXAPIAN_REALLY_NO_DEBUG_LOG\
	-I$(top_srcdir)/backends/glass
bin_xapian_inspect_SOURCES = bin/xapian-inspect.cc\
	api/error.cc\
	backends/glass/glass_changes.cc\
	backends/glass/glass_cursor.cc\
	backends/glass/glass_freelist.cc\
	backends/glass/glass_table.cc\
	backends/glass/glass_version.cc\
	common/compression_stream.cc\
	common/errno_to_string.cc\
	common/io_utils.cc\
	common/posixy_wrapper.cc\
	common/str.cc\
	unicode/description_append.cc\
	unicode/unicode-data.cc\
	unicode/utf8itor.cc

# XAPIAN_LIBS gives us zlib and any library needed for UUIDs.
bin_xapian_inspect_LDADD = $(ldflags) libgetopt.la $(XAPIAN_LIBS)
if USE_PROC_FOR_UUID
bin_xapian_inspect_SOURCES +=\
	api/constinfo.cc\
	common/proc_uuid.cc
endif
if USE_WIN32_UUID_API
bin_xapian_inspect_SOURCES +=\
	common/win32_uuid.cc
bin_xapian_inspect_LDADD += -lrpcrt4
endif

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

bin/xapian-progsrv.1: bin/xapian-progsrv$(EXEEXT) makemanpage
	./makemanpage bin/xapian-progsrv $(srcdir)/bin/xapian-progsrv.cc bin/xapian-progsrv.1

bin/xapian-replicate.1: bin/xapian-replicate$(EXEEXT) makemanpage
	./makemanpage bin/xapian-replicate $(srcdir)/bin/xapian-replicate.cc bin/xapian-replicate.1

bin/xapian-replicate-server.1: bin/xapian-replicate-server$(EXEEXT) makemanpage
	./makemanpage bin/xapian-replicate-server $(srcdir)/bin/xapian-replicate-server.cc bin/xapian-replicate-server.1

bin/xapian-tcpsrv.1: bin/xapian-tcpsrv$(EXEEXT) makemanpage
	./makemanpage bin/xapian-tcpsrv $(srcdir)/bin/xapian-tcpsrv.cc bin/xapian-tcpsrv.1
endif
