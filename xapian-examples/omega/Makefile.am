## Process this file with automake to produce Makefile.in

noinst_HEADERS = omega.h query.h cgiparam.h parsequery.h symboltab.h\
 htmlparse.h utils.h configfile.h

# include yacc/bison generated C++ source
EXTRA_DIST = parsequery.cc templates/query templates/godmode

INCLUDES = $(XAPIAN_CFLAGS)

if HAVE_LIBXML2
XML2_PROGS=omindex-config
else
XML2_PROGS=
endif

bin_PROGRAMS = omega omindex $(XML2_PROGS)

TESTS = parsequerytest

check_PROGRAMS = parsequerytest

omega_SOURCES = omega.cc query.cc cgiparam.cc parsequery.yy utils.cc \
		configfile.cc
omega_LDADD = @XAPIAN_LIBS@

omindex_SOURCES = omindex.cc htmlparse.cc
omindex_LDADD = @XAPIAN_LIBS@

if HAVE_LIBXML2
omindex_config_SOURCES = omindex-config.cc
#per-target CXXFLAGS not supported in older versions of automake
#omindex_config_CXXFLAGS = $(LIBXML_CFLAGS)
AM_CXXFLAGS = $(LIBXML_CFLAGS)
omindex_config_LDADD = $(LIBXML_LIBS)
endif

parsequerytest_SOURCES = parsequerytest.cc parsequery.yy
parsequerytest_LDADD = @XAPIAN_LIBS@
