## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I/usr/local/BerkeleyDB/include

noinst_HEADERS = sleepy_list.h \
                 sleepy_postlist.h \
		 sleepy_termlist.h \
		 sleepy_termcache.h \
                 sleepy_database.h \
                 sleepy_database_internals.h

noinst_LTLIBRARIES = libsleepy.la

libsleepy_la_SOURCES = sleepy_list.cc \
                       sleepy_postlist.cc \
                       sleepy_termlist.cc \
                       sleepy_termcache.cc \
                       sleepy_database.cc \
                       sleepy_database_internals.cc
libsleepy_la_LIBADD = -L/usr/local/BerkeleyDB/lib -ldb
