## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
           @BERKELEY_2_x_INCLUDES@

noinst_HEADERS = sleepycat_list.h \
                 sleepycat_postlist.h \
		 sleepycat_termlist.h \
		 sleepycat_termcache.h \
                 sleepycat_database.h \
                 sleepycat_database_internals.h \
		 sleepycat_document.h

noinst_LTLIBRARIES = libsleepycat.la

libsleepycat_la_SOURCES = sleepycat_list.cc \
                       sleepycat_postlist.cc \
                       sleepycat_termlist.cc \
                       sleepycat_termcache.cc \
                       sleepycat_database.cc \
                       sleepycat_database_internals.cc \
		       sleepycat_document.cc

libsleepycat_la_LIBADD = @BERKELEY_2_x_LIBS@

libsleepycat_la_LDFLAGS = @BERKELEY_2_x_LINKFLAGS@
