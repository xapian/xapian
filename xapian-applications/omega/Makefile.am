## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.6.3

noinst_HEADERS = omega.h query.h cgiparam.h symboltab.h\
 htmlparse.h utils.h configfile.h date.h indextext.h\
 getopt.h

EXTRA_DIST = templates/query \
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
	docs/cgiparams.txt \
	docs/omegascript.txt \
	docs/overview.txt \
	docs/quickstart.txt \
	docs/scriptindex.txt \
	dbi2omega htdig2omega htdig2omega.script

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS)

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega
bin_PROGRAMS = omindex scriptindex
bin_SCRIPTS = dbi2omega htdig2omega

# Data to install, in the usual automake way
docdatadir   = $(datadir)/doc/omega
docdata_DATA = docs/cgiparams.txt \
	docs/omegascript.txt \
	docs/overview.txt \
	docs/quickstart.txt \
	docs/scriptindex.txt

omegadatadir = $(datadir)/omega
omegadata_DATA = htdig2omega.script

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc
omega_LDADD = $(XAPIAN_LIBS) -lxapianqueryparser # -lpcre

omindex_SOURCES = omindex.cc htmlparse.cc indextext.cc\
 getopt.c getopt1.c
omindex_LDADD = $(XAPIAN_LIBS)

scriptindex_SOURCES = scriptindex.cc htmlparse.cc indextext.cc\
 getopt.c getopt1.c
scriptindex_LDADD = $(XAPIAN_LIBS)
