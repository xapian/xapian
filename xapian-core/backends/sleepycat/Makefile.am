## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
           @BERKELEY_2_x_INCLUDES@

noinst_HEADERS = sleepy_list.h \
                 sleepy_postlist.h \
		 sleepy_termlist.h \
		 sleepy_termcache.h \
                 sleepy_database.h \
                 sleepy_database_internals.h \
		 sleepy_document.h

noinst_LTLIBRARIES = libsleepy.la

libsleepy_la_SOURCES = sleepy_list.cc \
                       sleepy_postlist.cc \
                       sleepy_termlist.cc \
                       sleepy_termcache.cc \
                       sleepy_database.cc \
                       sleepy_database_internals.cc \
		       sleepy_document.cc

libsleepy_la_LIBADD = @BERKELEY_2_x_LIBS@

libsleepy_la_LDFLAGS = @BERKELEY_2_x_LINKFLAGS@
