## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = da_document.h damuscat.h daread.h da_database.h

noinst_LTLIBRARIES = libda.la

libda_la_SOURCES = da_database.cc da_document.cc da_record.cc
