## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.6.3

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS)

bin_PROGRAMS = xapian-queryserver

php4dir = $(pkglibdir)/php4
dist_php4_SCRIPTS = clients/index.php clients/xapian-queryclient.php

# Data to install, in the usual automake way
docdatadir   = $(datadir)/doc/xapian-queryserver
dist_docdata_DATA = \
    docs/xapian-queryserver.but \
    docs/xapian-queryserver.txt

noinst_HEADERS = source/base64.h
xapian_queryserver_SOURCES = source/queryserver.cpp source/base64.c
xapian_queryserver_LDADD = $(XAPIAN_LIBS)

docs/xapian-queryserver.txt: docs/xapian-queryserver.but
	$(mkinstalldirs) docs
	$(HALIBUT) --text=$@.tmp $^
	mv $@.tmp $@

# Build documentation
all-local: docs/xapian-queryserver.txt

