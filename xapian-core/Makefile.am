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

bin_SCRIPTS=libomus-config

CLEANFILES=TODO libomus-config

EXTRA_DIST = Makefile_common.am ChangeLog buildall HACKING PLATFORMS \
             ar-wrapper-solaris TODO

lib_LTLIBRARIES = libomus.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libomus_la_SOURCES = dummy.c
EXTRA_libomus_la_SOURCES =

TODO: ./docs/todo2html.pl $(top_srcdir)/docs/todo.xml
if HAVE_PERL_XML
	@echo "Generating TODO"
	@./docs/todo2html.pl --text $(top_srcdir)/docs/todo.xml > TODO
else
	@echo "Can't generate TODO: need Perl XML module."
	@exit 1
endif

acinclude.m4: autoconf/acinclude.m4
	cp autoconf/acinclude.m4 acinclude.m4

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

libomus-config: libomus-config.nodep libomus.la
	@echo "Creating $@"
	@export dependency_libs; \
	. $(top_builddir)/libomus.la ; \
	cat $< | sed "s%@dependency_libs@%$${dependency_libs}%" > $@
