## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
           -I$(top_srcdir)/languages

noinst_HEADERS = omdatabaseinternal.h ompostlistiteratorinternal.h \
 ompositionlistiteratorinternal.h omtermlistiteratorinternal.h
noinst_LTLIBRARIES = libapi.la

libapi_la_SOURCES = omenquire.cc omquery.cc omdatabase.cc omstem.cc \
		    ombatchenquire.cc omdocument.cc omindexdoc.cc \
		    omdatabaseinternal.cc omerror.cc omsettings.cc \
		    ompostlistiterator.cc ompositionlistiterator.cc \
		    omtermlistiterator.cc
