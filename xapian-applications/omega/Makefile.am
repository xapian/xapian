## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 foreign # don't fuss about GNU required files

noinst_HEADERS = omega.h query.h cgiparam.h symboltab.h\
 htmlparse.h utils.h configfile.h

EXTRA_DIST = templates/query templates/godmode

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
