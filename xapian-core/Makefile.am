## Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CFLAGS, etc; also so generated files in EXTRA_DIST work
AUTOMAKE_OPTIONS = 1.5

#ln -s `find .. -type f \( -name \*.cc -o -name \*.h -o -name \*.c \) -print` .

if BUILD_BACKEND_REMOTE
BUILDSUBDIR_REMOTE1 = net
BUILDSUBDIR_REMOTE2 = netprogs
LIB_REMOTE1 = net/libnet.la
endif

# Order is relvant: when building, tests must be at end
# When installing, should install script and m4 stuff last, in case of failure
SUBDIRS = include testsuite autoconf common \
          api backends matcher languages $(BUILDSUBDIR_REMOTE1) \
	  . m4 docs $(BUILDSUBDIR_REMOTE2) tests extra
# Disabled for now: bindings

DIST_SUBDIRS = include testsuite autoconf common \
               api backends matcher languages net \
	       . m4 docs netprogs tests extra
# Disabled for now: bindings

bin_SCRIPTS = xapian-config

CLEANFILES = TODO TODO.release xapian-config

EXTRA_DIST = Makefile_common.am ChangeLog buildall HACKING PLATFORMS \
             ar-wrapper-solaris TODO TODO.release

lib_LTLIBRARIES = libxapian.la

# This line is present because of a bug in some versions of libtool
# (exhibited when there are no source files)
libxapian_la_SOURCES = dummy.c
EXTRA_libxapian_la_SOURCES =

TODO: $(srcdir)/docs/todo2html.pl $(srcdir)/docs/todo.xml
if HAVE_PERL_XML_AND_TEXT
	$(PERL) $(srcdir)/docs/todo2html.pl --text $(srcdir)/docs/todo.xml >$@
else
	@echo "Can't generate $@: need Perl XML and Text::Format module."
	false
endif

TODO.release: $(srcdir)/docs/todo2html.pl $(srcdir)/docs/todo.xml
if HAVE_PERL_XML_AND_TEXT
	$(PERL) $(srcdir)/docs/todo2html.pl --release \* --text $(srcdir)/docs/todo.xml >$@
else
	@echo "Can't generate $@: need Perl XML and Text::Format module."
	false
endif

# http://ctags.sourceforge.net/
ctags:
	ctags-exuberant -R --links=no

# doesn't really work as we'd hope as we get to play with the mangled symbols
libxapian_la_LDFLAGS = -export-symbols-regex 'Om[A-Z]'

libxapian_la_LIBADD = \
 common/libcommon.la \
 backends/libbackend.la \
 matcher/libmatcher.la \
 languages/liblanguages.la \
 api/libapi.la \
 $(LIB_REMOTE1)

xapian-config: xapian-config.nodep libxapian.la
	@echo "Creating $@"
	export dependency_libs; \
	. $(top_builddir)/libxapian.la ; \
	sed "s%@dependency_libs@%$${dependency_libs}%" < xapian-config.nodep > $@
