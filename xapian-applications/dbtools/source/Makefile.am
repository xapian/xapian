## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) $(LIBXML_CFLAGS)

bin_PROGRAMS = xdb-manage xdb-dump

xdb_manage_SOURCES = xdb-manage.cc xdb-options.cc
xdb_manage_LDADD = $(XAPIAN_LIBS) $(LIBXML_LIBS)

xdb_dump_SOURCES = xdb-dump.cc xdb-options.cc
xdb_dump_LDADD = $(XAPIAN_LIBS) $(LIBXML_LIBS)
