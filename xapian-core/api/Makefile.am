## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
	   -I$(top_builddir)/include

noinst_HEADERS = maptermlist.h vectortermlist.h
noinst_LTLIBRARIES = libapi.la

libapi_la_SOURCES = errorhandler.cc omenquire.cc omquery.cc omqueryinternal.cc \
		    omdatabase.cc omdocument.cc \
		    ompostlistiterator.cc ompositionlistiterator.cc \
		    omtermlistiterator.cc omvalueiterator.cc version.cc
