## Process this file with automake to produce Makefile.in

noinst_HEADERS = omega.h query.h cgiparam.h parsequery.h symboltab.h\
 htmlparse.h utils.h

# include yacc/bison generated C++ source
EXTRA_DIST = parsequery.cc templates/query templates/godmode

INCLUDES = $(OMSEE_CFLAGS)

bin_PROGRAMS = omega omindex # omindex-config

TESTS = parsequerytest

check_PROGRAMS = parsequerytest

omega_SOURCES = omega.cc query.cc cgiparam.cc parsequery.yy utils.cc
omega_LDADD = @OMSEE_LIBS@

omindex_SOURCES = omindex.cc htmlparse.cc
omindex_LDADD = @OMSEE_LIBS@

# omindex_config_SOURCES = omindex-config.cc
# #per-target CXXFLAGS not supported in older versions of automake
# #omindex_config_CXXFLAGS = $(LIBXML_CFLAGS)
# AM_CXXFLAGS += $(LIBXML_CFLAGS)
# omindex_config_LDADD = $(LIBXML_LIBS)

parsequerytest_SOURCES = parsequerytest.cc parsequery.yy
parsequerytest_LDADD = @OMSEE_LIBS@
