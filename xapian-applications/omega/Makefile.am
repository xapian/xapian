## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5

noinst_HEADERS = omega.h query.h cgiparam.h symboltab.h\
 htmlparse.h utils.h configfile.h

EXTRA_DIST = NEWS README TODO \
	templates/query \
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
	docs/quickstart.txt

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS)

# if HAVE_LIBXML2
# XML2_PROGS=omindex-config
# else
# XML2_PROGS=
# endif

bin_PROGRAMS = omega omindex # $(XML2_PROGS)

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc
omega_LDADD = $(XAPIAN_LIBS) -lomqueryparser

omindex_SOURCES = omindex.cc htmlparse.cc
omindex_LDADD = $(XAPIAN_LIBS)

# if HAVE_LIBXML2
# omindex_config_SOURCES = omindex-config.cc
# omindex_config_CXXFLAGS = $(LIBXML_CFLAGS)
# omindex_config_LDADD = $(LIBXML_LIBS)
# endif
