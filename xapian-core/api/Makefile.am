## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
           -I$(top_srcdir)/languages

noinst_HEADERS = omdatabaseinternal.h omenquireinternal.h

noinst_LTLIBRARIES = libapi.la

libapi_la_SOURCES = omenquire.cc omquery.cc omdatabase.cc \
		    omstem.cc ombatchenquire.cc
