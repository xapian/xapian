## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS = textfile_database.h

noinst_LTLIBRARIES = libtextfile.la

libtextfile_la_SOURCES = textfile_database.cc \
    textfile_document.h textfile_document.cc
