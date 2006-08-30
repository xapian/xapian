## Process this file with automake to produce Makefile.in

EXTRA_DIST = templates/query \
	templates/opensearch \
	templates/godmode \
	templates/xml \
	templates/anyalldropbox \
	templates/anyallradio \
	templates/toptermsjs \
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
	omega.spec

AM_CPPFLAGS = -DCONFIGFILE_SYSTEM=\"$(sysconfdir)/omega.conf\"

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega
bin_PROGRAMS = omindex scriptindex
dist_bin_SCRIPTS = dbi2omega htdig2omega mbox2omega

# Install documentation.
docdatadir   = $(datadir)/doc/omega
dist_docdata_DATA = \
	docs/cgiparams.txt \
	docs/omegascript.txt \
	docs/overview.txt \
	docs/quickstart.txt \
	docs/scriptindex.txt \
	docs/termprefixes.txt

omegadatadir = $(datadir)/omega
dist_omegadata_DATA = htdig2omega.script mbox2omega.script

dist_sysconf_DATA = omega.conf

noinst_HEADERS = omega.h query.h cgiparam.h symboltab.h\
 myhtmlparse.h htmlparse.h utils.h configfile.h date.h indextext.h\
 gnu_getopt.h commonhelp.h cdb.h cdb_int.h hashterm.h loadfile.h

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc\
 cdb_init.cc cdb_find.cc cdb_hash.cc cdb_unpack.cc loadfile.cc
omega_LDADD = $(XAPIAN_LIBS) # -lpcre

omindex_SOURCES = omindex.cc myhtmlparse.cc htmlparse.cc indextext.cc\
 getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc
omindex_LDADD = $(XAPIAN_LIBS)

scriptindex_SOURCES = scriptindex.cc myhtmlparse.cc htmlparse.cc indextext.cc\
 getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc
scriptindex_LDADD = $(XAPIAN_LIBS)

dist_man_MANS = omindex.1 scriptindex.1

MAINTAINERCLEANFILES = $(dist_man_MANS)

if MAINTAINER_MODE
CLEANFILES = $(dist_man_MANS)

omindex.1: omindex
	$(HELP2MAN) -o omindex.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/omindex.cc`" ./omindex

scriptindex.1: scriptindex
	$(HELP2MAN) -o scriptindex.1 --no-info -S "$(PACKAGE_STRING)" -n "`sed 's/^#define *PROG_DESC *\"\(.*\)\".*/\1/p;d' $(srcdir)/scriptindex.cc`" ./scriptindex
endif
