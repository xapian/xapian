## Process this file with automake to produce Makefile.in

#ln -s `find .. -type f \( -name \*.cc -o -name \*.h -o -name \*.c \) -print` .

if BUILD_BACKEND_REMOTE
BUILDSUBDIR_REMOTE1 = net
BUILDSUBDIR_REMOTE2 = netprogs
LIB_REMOTE1 = net/libnet.la
endif

if BUILD_INDEXER
BUILDSUBDIR_INDEXER = indexer
LIB_INDEXER = indexer/libindexer.la
endif

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include testsuite autoconf common \
          api backends matcher languages $(BUILDSUBDIR_REMOTE1) \
	  $(BUILDSUBDIR_INDEXER) . m4 \
	  docs $(BUILDSUBDIR_REMOTE2) tests bindings

DIST_SUBDIRS = include testsuite autoconf common \
               api backends matcher languages net \
	       indexer  . m4 docs netprogs tests bindings

bin_SCRIPTS=omsee-config

CLEANFILES=TODO omsee-config

EXTRA_DIST = Makefile_common.am ChangeLog buildall HACKING PLATFORMS \
             ar-wrapper-solaris TODO TODO.release

lib_LTLIBRARIES = libomsee.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libomsee_la_SOURCES = dummy.c
EXTRA_libomsee_la_SOURCES =

TODO: ./docs/todo2html.pl $(top_srcdir)/docs/todo.xml $(top_srcdir)/docs/todo2html.pl.in
if HAVE_PERL_XML
	@echo "Generating $@"
	@./docs/todo2html.pl --text $(top_srcdir)/docs/todo.xml > $@
else
	@echo "Can't generate $@: need Perl XML module."
	@exit 0
endif

TODO.release: ./docs/todo2html.pl $(top_srcdir)/docs/todo.xml $(top_srcdir)/docs/todo2html.pl.in
if HAVE_PERL_XML
	@echo "Generating $@"
	@./docs/todo2html.pl --release \* --text $(top_srcdir)/docs/todo.xml > $@
else
	@echo "Can't generate $@: need Perl XML module."
	@exit 0
endif

# http://ctags.sourceforge.net/
ctags:
	ctags-exuberant -R --links=no

acinclude.m4: autoconf/acinclude.m4
	cp autoconf/acinclude.m4 acinclude.m4

# doesn't really work as we'd hope as we get to play with the mangled symbols
libomsee_la_LDFLAGS = -export-symbols-regex 'Om[A-Z]'

libomsee_la_LIBADD = \
 common/libcommon.la \
 backends/libbackend.la \
 matcher/libmatcher.la \
 languages/liblanguages.la \
 $(LIB_INDEXER) \
 api/libapi.la \
 $(LIB_REMOTE1)

omsee-config: omsee-config.nodep libomsee.la
	@echo "Creating $@"
	@export dependency_libs; \
	. $(top_builddir)/libomsee.la ; \
	cat $< | sed "s%@dependency_libs@%$${dependency_libs}%" > $@
