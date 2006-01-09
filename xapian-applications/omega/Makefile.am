## Process this file with automake to produce Makefile.in

# Need 1.5 for AM_CXXFLAGS, etc; 1.6.3 for AM_INIT_AUTOMAKE with no args
# 1.8.5 contains a lot of fixes over 1.6.3
AUTOMAKE_OPTIONS = 1.8.5

noinst_HEADERS = omega.h query.h cgiparam.h symboltab.h\
 myhtmlparse.h htmlparse.h utils.h configfile.h date.h indextext.h\
 gnu_getopt.h commonhelp.h cdb.h cdb_int.h

# Non-generated documentation.
omega_docs = docs/cgiparams.txt \
	docs/omegascript.txt \
	docs/overview.txt \
	docs/quickstart.txt \
	docs/scriptindex.txt \
	docs/termprefixes.txt

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
	omega.spec omega.conf \
	$(omega_docs) \
	dbi2omega htdig2omega htdig2omega.script mbox2omega mbox2omega.script

AM_CPPFLAGS = -DCONFIGFILE_SYSTEM=\"@sysconfdir@/omega.conf\"

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega
bin_PROGRAMS = omindex scriptindex
bin_SCRIPTS = dbi2omega htdig2omega mbox2omega

# Install documentation.
docdatadir   = $(datadir)/doc/omega
docdata_DATA = $(omega_docs)

omegadatadir = $(datadir)/omega
omegadata_DATA = htdig2omega.script mbox2omega.script

sysconf_DATA = omega.conf

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc\
 cdb_init.cc cdb_find.cc cdb_hash.cc cdb_unpack.cc
omega_LDADD = $(XAPIAN_LIBS) # -lpcre

omindex_SOURCES = omindex.cc myhtmlparse.cc htmlparse.cc indextext.cc\
 getopt.cc commonhelp.cc utils.cc
omindex_LDADD = $(XAPIAN_LIBS)

scriptindex_SOURCES = scriptindex.cc myhtmlparse.cc htmlparse.cc indextext.cc\
 getopt.cc commonhelp.cc utils.cc
scriptindex_LDADD = $(XAPIAN_LIBS)
