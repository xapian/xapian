## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libinmemory.la

if BUILD_BACKEND_INMEMORY
noinst_HEADERS = inmemory_database.h inmemory_document.h \
	inmemory_alltermslist.h

libinmemory_la_SOURCES = inmemory_database.cc inmemory_document.cc \
    inmemory_positionlist.cc inmemory_alltermslist.cc
else
libinmemory_la_SOURCES = inmemory_positionlist.cc
endif
