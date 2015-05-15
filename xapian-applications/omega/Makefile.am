## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . docs

BUILT_SOURCES = extra/omegascript.vim

EXTRA_DIST =\
	extra/omegascript.vim \
	extra/omegascript.vim.in \
	gen-omegascript-vim \
	templates/query \
	templates/topterms \
	templates/opensearch \
	templates/godmode \
	templates/xml \
	templates/inc/anyalldropbox \
	templates/inc/anyallradio \
	templates/inc/toptermsjs \
	images/nextoff.png \
	images/next.png \
	images/page-10.png \
	images/page-10s.png \
	images/page-11.png \
	images/page-11s.png \
	images/page-1.png \
	images/page-1s.png \
	images/page-2.png \
	images/page-2s.png \
	images/page-3.png \
	images/page-3s.png \
	images/page-4.png \
	images/page-4s.png \
	images/page-5.png \
	images/page-5s.png \
	images/page-6.png \
	images/page-6s.png \
	images/page-7.png \
	images/page-7s.png \
	images/page-8.png \
	images/page-8s.png \
	images/page-9.png \
	images/page-9s.png \
	images/previous.png \
	images/prevoff.png \
	images/score-0.png \
	images/score-10.png \
	images/score-1.png \
	images/score-2.png \
	images/score-3.png \
	images/score-4.png \
	images/score-5.png \
	images/score-6.png \
	images/score-7.png \
	images/score-8.png \
	images/score-9.png \
	xapian-omega.spec

AM_CPPFLAGS = \
	-I$(top_srcdir)/common \
	-DCONFIGFILE_SYSTEM=\"$(sysconfdir)/omega.conf\" \
	-DPKGLIBBINDIR=\"$(pkglibbindir)\"
AM_CXXFLAGS += $(XAPIAN_CXXFLAGS)

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega
dist_pkglibbin_SCRIPTS = outlookmsg2html
bin_PROGRAMS = omindex scriptindex
dist_bin_SCRIPTS = dbi2omega htdig2omega mbox2omega

check_PROGRAMS = atomparsetest htmlparsetest md5test urlenctest utf8converttest
TESTS =	atomparsetest$(EXEEXT)\
	htmlparsetest$(EXEEXT)\
	md5test$(EXEEXT)\
	urlenctest$(EXEEXT)\
	utf8converttest$(EXEEXT)

omegadatadir = $(datadir)/omega
dist_omegadata_DATA = htdig2omega.script mbox2omega.script

dist_sysconf_DATA = omega.conf

noinst_HEADERS = omega.h query.h cgiparam.h\
 myhtmlparse.h htmlparse.h utils.h configfile.h date.h\
 commonhelp.h cdb.h cdb_int.h hashterm.h loadfile.h\
 md5.h md5wrap.h xmlparse.h metaxmlparse.h values.h utf8convert.h\
 namedentities.h pkglibbindir.h datematchdecider.h sample.h strcasecmp.h\
 utf8truncate.h diritor.h runfilter.h freemem.h xpsxmlparse.h transform.h\
 weight.h expand.h svgparse.h urldecode.h urlencode.h unixperm.h atomparse.h\
 xlsxparse.h opendocparse.h\
 portability/mkdtemp.h

# headers maintained in xapian-core
noinst_HEADERS +=\
	common/append_filename_arg.h\
	common/gnu_getopt.h\
	common/msvc_dirent.h\
	common/noreturn.h\
	common/omassert.h\
	common/realtime.h\
	common/safedirent.h\
	common/safeerrno.h\
	common/safefcntl.h\
	common/safesysselect.h\
	common/safesysstat.h\
	common/safesyswait.h\
	common/safeunistd.h\
	common/safewindows.h\
	common/safewinsock2.h\
	common/str.h\
	common/stringutils.h

AM_LDFLAGS = $(NO_UNDEFINED) $(ICONV_LDFLAGS)

# We want to compile transform.cc with PCRE_CFLAGS, but if this adds a -I for
# a directory with a different version of iconv, then we get problems (this
# has been reported on Mac OS X).  So we only add PCRE_CFLAGS when compiling
# transform.cc, using the convenience library technique suggested by the
# automake manual.
noinst_LTLIBRARIES = libtransform.la
libtransform_la_SOURCES = transform.cc
libtransform_la_CPPFLAGS = $(PCRE_CFLAGS)
libtransform_la_LIBADD = $(PCRE_LIBS)

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc\
 cdb_init.cc cdb_find.cc cdb_hash.cc cdb_unpack.cc loadfile.cc\
 datematchdecider.cc common/str.cc urlencode.cc weight.cc expand.cc
# Not currently used, and we ought to handle non-Unix perms too: unixperm.cc
omega_LDADD = $(XAPIAN_LIBS) libtransform.la

omindex_SOURCES = omindex.cc myhtmlparse.cc htmlparse.cc\
 common/getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc md5.cc\
 md5wrap.cc xmlparse.cc metaxmlparse.cc utf8convert.cc sample.cc diritor.cc\
 runfilter.cc freemem.cc common/msvc_dirent.cc xpsxmlparse.cc common/str.cc\
 pkglibbindir.cc svgparse.cc urlencode.cc atomparse.cc xlsxparse.cc\
 opendocparse.cc
if NEED_MKDTEMP
omindex_SOURCES += portability/mkdtemp.cc
endif
omindex_LDADD = $(MAGIC_LIBS) $(XAPIAN_LIBS)

scriptindex_SOURCES = scriptindex.cc myhtmlparse.cc htmlparse.cc\
 common/getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc\
 common/safe.cc common/stringutils.cc utf8convert.cc utf8truncate.cc
scriptindex_LDADD = $(XAPIAN_LIBS)

atomparsetest_SOURCES = atomparsetest.cc atomparse.cc htmlparse.cc\
 myhtmlparse.cc utf8convert.cc
atomparsetest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS)

htmlparsetest_SOURCES = htmlparsetest.cc myhtmlparse.cc htmlparse.cc\
 utf8convert.cc
htmlparsetest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS)

md5test_SOURCES = md5test.cc md5wrap.cc md5.cc
md5test_LDADD = $(NO_INSTALL)

utf8converttest_SOURCES = utf8converttest.cc utf8convert.cc
utf8converttest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS)

urlenctest_SOURCES = urlenctest.cc urlencode.cc common/stringutils.cc
urlenctest_LDADD = $(NO_INSTALL)

if !MAINTAINER_NO_DOCS
dist_man_MANS = omindex.1 scriptindex.1
MAINTAINERCLEANFILES = $(dist_man_MANS)
endif

if DOCUMENTATION_RULES
omindex.1: omindex$(EXEEXT) makemanpage
	./makemanpage ./omindex $(srcdir)/omindex.cc omindex.1

scriptindex.1: scriptindex$(EXEEXT) makemanpage
	./makemanpage ./scriptindex $(srcdir)/scriptindex.cc scriptindex.1
endif

if MAINTAINER_MODE
extra/omegascript.vim: extra/omegascript.vim.in query.cc configure.ac
	$(PERL) $(srcdir)/gen-omegascript-vim "$(srcdir)" "$(PACKAGE_VERSION)"
endif

DISTCHECK_CONFIGURE_FLAGS = "XAPIAN_CONFIG=$(XAPIAN_CONFIG)"
