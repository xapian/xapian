## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_builddir)/include

noinst_LTLIBRARIES = libquartz.la libbtreecheck.la

noinst_HEADERS =       quartz_types.h \
                       quartz_database.h \
		       quartz_termlist.h \
		       quartz_postlist.h \
		       quartz_positionlist.h \
		       quartz_record.h \
		       quartz_values.h \
		       quartz_utils.h \
		       quartz_log.h \
		       quartz_document.h \
		       quartz_alltermslist.h \
		       quartz_metafile.h \
		       btree.h \
		       bcursor.h \
		       btree_util.h \
		       btree_base.h \
		       btreecheck.h

libquartz_la_SOURCES = quartz_database.cc \
		       quartz_termlist.cc \
		       quartz_postlist.cc \
		       quartz_positionlist.cc \
		       quartz_record.cc \
		       quartz_values.cc \
		       quartz_log.cc \
		       quartz_document.cc \
		       quartz_alltermslist.cc \
		       quartz_metafile.cc \
		       btree.cc \
		       bcursor.cc \
		       btree_base.cc

libbtreecheck_la_SOURCES = btreecheck.cc
libbtreecheck_la_LDFLAGS = ../../libxapian.la
