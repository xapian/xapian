## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS = inmemory_database.h inmemory_document.h

noinst_LTLIBRARIES = libinmemory.la

libinmemory_la_SOURCES = inmemory_database.cc inmemory_document.cc
