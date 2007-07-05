## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.6.3

SUBDIRS = . docs
DIST_SUBDIRS = . docs

bin_PROGRAMS = xapian-queryserver

php4dir = $(pkglibdir)/php4
dist_php4_SCRIPTS = clients/index.php clients/xapian-queryclient.php

noinst_HEADERS = source/base64.h
xapian_queryserver_SOURCES = source/queryserver.cpp source/base64.cc
xapian_queryserver_LDADD = $(XAPIAN_LIBS)
