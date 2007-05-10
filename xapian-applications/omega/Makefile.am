## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

INCLUDES = -I$(top_srcdir)/common

EXTRA_DIST =\
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

AM_CPPFLAGS = -DCONFIGFILE_SYSTEM=\"$(sysconfdir)/omega.conf\"

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega
bin_PROGRAMS = omindex scriptindex
dist_bin_SCRIPTS = dbi2omega htdig2omega mbox2omega

check_PROGRAMS = htmlparsetest md5test
TESTS = htmlparsetest$(EXEEXT) md5test$(EXEEXT)

# Install documentation.
dist_doc_DATA = \
	docs/cgiparams.txt \
	docs/omegascript.txt \
	docs/overview.txt \
	docs/quickstart.txt \
	docs/scriptindex.txt \
	docs/termprefixes.txt

omegadatadir = $(datadir)/omega
dist_omegadata_DATA = htdig2omega.script mbox2omega.script

dist_sysconf_DATA = omega.conf

noinst_HEADERS = omega.h query.h cgiparam.h\
 myhtmlparse.h htmlparse.h utils.h configfile.h date.h\
 commonhelp.h cdb.h cdb_int.h hashterm.h loadfile.h\
 md5.h md5wrap.h xmlparse.h metaxmlparse.h values.h utf8convert.h\
 namedentities.h datematchdecider.h sample.h strcasecmp.h\
 utf8truncate.h

# headers maintained in xapian-core
noinst_HEADERS +=\
	common/gnu_getopt.h\
	common/safeerrno.h\
	common/safefcntl.h\
	common/safesysselect.h\
	common/safesysstat.h\
	common/safeunistd.h\
	common/safewindows.h

AM_LDFLAGS = $(ICONV_LDFLAGS)

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc\
 cdb_init.cc cdb_find.cc cdb_hash.cc cdb_unpack.cc loadfile.cc\
 utf8convert.cc datematchdecider.cc
omega_LDADD = $(XAPIAN_LIBS) # -lpcre

omindex_SOURCES = omindex.cc myhtmlparse.cc htmlparse.cc\
 common/getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc md5.cc\
 md5wrap.cc xmlparse.cc metaxmlparse.cc utf8convert.cc sample.cc
if NEED_MKDTEMP
omindex_SOURCES += portability/mkdtemp.cc
endif
omindex_LDADD = $(XAPIAN_LIBS)

scriptindex_SOURCES = scriptindex.cc myhtmlparse.cc htmlparse.cc\
 common/getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc\
 utf8convert.cc utf8truncate.cc
scriptindex_LDADD = $(XAPIAN_LIBS)

htmlparsetest_SOURCES = htmlparsetest.cc myhtmlparse.cc htmlparse.cc\
 utf8convert.cc
htmlparsetest_LDADD = $(XAPIAN_LIBS)

md5test_SOURCES = md5test.cc md5wrap.cc md5.cc

dist_man_MANS = omindex.1 scriptindex.1

MAINTAINERCLEANFILES = $(dist_man_MANS)

if MAINTAINER_MODE
CLEANFILES = $(dist_man_MANS)

omindex.1: omindex$(EXEEXT) makemanpage
	./makemanpage ./omindex $(srcdir)/omindex.cc omindex.1

scriptindex.1: scriptindex$(EXEEXT) makemanpage
	./makemanpage ./scriptindex $(srcdir)/scriptindex.cc scriptindex.1
endif
