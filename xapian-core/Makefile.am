## Process this file with automake to produce Makefile.in

if BUILD_BACKEND_REMOTE
BUILDSUBDIR_REMOTE1 = net
BUILDSUBDIR_REMOTE2 = netprogs
LIB_REMOTE1 = net/libnet.la
endif

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include testsuite autoconf common \
          backends api matcher languages $(BUILDSUBDIR_REMOTE1) indexer . m4 \
	  docs $(BUILDSUBDIR_REMOTE2) tests bindings

EXTRA_DIST = Makefile_common.am ChangeLog buildall HACKING PLATFORMS \
             ar-wrapper-solaris

if HAVE_PERL_XML
dist-hook: todo_dist 
else
dist-hook:
endif

lib_LTLIBRARIES = libomus.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libomus_la_SOURCES = dummy.c
EXTRA_libomus_la_SOURCES =

todo_dist: TODO
	@echo "Adding TODO to distribution"
	cp TODO $(distdir)/

TODO: docs/todo2html.pl
	@echo "Generating TODO"
	@./docs/todo2html.pl --text $(top_srcdir)/docs/todo.xml > TODO

acinclude.m4: @m4_files@ configure.in
	./genacinclude.sh @m4_files@

# doesn't really work as we'd hope as we get to play with the mangled symbols
libomus_la_LDFLAGS = -export-symbols-regex 'Om[A-Z]'

libomus_la_LIBADD = \
 common/libcommon.la \
 backends/libbackend.la \
 matcher/libmatcher.la \
 languages/liblanguages.la \
 indexer/libindexer.la \
 api/libapi.la \
 $(LIB_REMOTE1)

bin_SCRIPTS = libomus-config

libomus-config: libomus-config.nodep libomus.la
	@echo "Creating $@"
	@export dependency_libs; \
	. $(top_builddir)/libomus.la ; \
	cat $< | sed "s%@dependency_libs@%$${dependency_libs}%" > $@
