## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite

noinst_HEADERS =       quartz_types.h \
                       quartz_database.h \
                       quartz_table_manager.h \
		       quartz_table_entries.h \
		       quartz_table.h \
		       quartz_termlist.h \
		       quartz_record.h \
		       quartz_attributes.h \
		       quartz_utils.h \
		       quartz_log.h
#		       quartz_postlist.h \
#		       quartz_positionlist.h \

libquartz_la_SOURCES = quartz_database.cc \
                       quartz_table_manager.cc \
		       quartz_table_entries.cc \
		       quartz_table.cc \
		       quartz_record.cc \
		       quartz_attributes.cc \
		       quartz_utils.cc \
		       quartz_log.cc
#		       quartz_postlist.cc \
#		       quartz_positionlist.cc \

# was after quartz_table.cc - richard failed to cvs add this so removed for
# now so snapshots work until he gets back
#		       quartz_termlist.cc \

noinst_LTLIBRARIES = libquartz.la

libquartz_la_LIBADD = @BERKELEY_3_1_LIBS@

libquartz_la_LDFLAGS = @BERKELEY_3_1_LINKFLAGS@

TESTS = quartztest

check_PROGRAMS = quartztest

quartztest_SOURCES = quartztest.cc
quartztest_LDADD = libquartz.la \
                   ../../testsuite/libtest.la \
		   ../../libomus.la

