## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
           @BERKELEY_3_1_INCLUDES@

noinst_HEADERS = quartz_database.h

noinst_LTLIBRARIES = libquartz.la

libquartz_la_SOURCES = quartz_database.cc

libquartz_la_LIBADD = @BERKELEY_3_1_LIBS@

libquartz_la_LDFLAGS = @BERKELEY_3_1_LINKFLAGS@
