## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = main.h query.h cgiparam.h symboltab.h

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = dbtest matchtest idxtest stemtest

ompf_SOURCES = main.cc query.cc cgiparam.cc
ompf_LDADD = ../libom.la
