## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS = da_record.h damuscat.h

noinst_LTLIBRARIES = libda.la

libda_la_SOURCES = da_database.cc daread.c da_record.cc
