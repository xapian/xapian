## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include \
           @BERKELEY_3_1_INCLUDES@

noinst_HEADERS =       quartz_types.h \
                       quartz_database.h \
                       quartz_db_manager.h \
		       quartz_modifications.h \
		       quartz_db_blocks.h \
		       quartz_db_diffs.h

libquartz_la_SOURCES = quartz_database.cc \
                       quartz_db_manager.cc \
		       quartz_modifications.cc \
		       quartz_db_blocks.cc \
		       quartz_db_diffs.cc

noinst_LTLIBRARIES = libquartz.la

libquartz_la_LIBADD = @BERKELEY_3_1_LIBS@

libquartz_la_LDFLAGS = @BERKELEY_3_1_LINKFLAGS@

TESTS = quartztest

check_PROGRAMS = quartztest

quartztest_SOURCES = quartztest.cc
quartztest_LDADD = ../../testsuite/libtest.la \
                   ../../common/libcommon.la \
		   ../libbackend.la \
		   libquartz.la

