## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

SUBDIRS = da sleepy textfile multi .

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS =

noinst_LTLIBRARIES = libbackend.la

libbackend_la_SOURCES = database_factory.cpp

libbackend_la_LIBADD = da/libda.la \
 sleepy/libsleepy.la \
 multi/libmulti.la \
 textfile/libtextfile.la
