## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite -I$(top_builddir)/include

noinst_HEADERS =       quartz_types.h \
                       quartz_database.h \
                       quartz_table_manager.h \
		       quartz_table_entries.h \
		       quartz_table.h \
		       quartz_termlist.h \
		       quartz_postlist.h \
		       quartz_positionlist.h \
		       quartz_lexicon.h \
		       quartz_record.h \
		       quartz_values.h \
		       quartz_utils.h \
		       quartz_log.h \
		       quartz_document.h \
		       quartz_alltermslist.h \
		       quartz_metafile.h \
		       btree.h \
		       btreecheck.h \
		       bcursor.h \
		       btree_types.h \
		       btree_util.h \
		       btree_base.h

libquartz_la_SOURCES = quartz_database.cc \
                       quartz_table_manager.cc \
		       quartz_table_entries.cc \
		       quartz_table.cc \
		       quartz_termlist.cc \
		       quartz_postlist.cc \
		       quartz_positionlist.cc \
		       quartz_lexicon.cc \
		       quartz_record.cc \
		       quartz_values.cc \
		       quartz_log.cc \
		       quartz_document.cc \
		       quartz_alltermslist.cc \
		       quartz_metafile.cc \
		       btree.cc \
		       bcursor.cc \
		       btree_base.cc

EXTRA_DIST = z_data/ord+ z_data/ord- z_data/ordnum+ z_data/ordnum-

noinst_LTLIBRARIES = libquartz.la

TESTS = btreetest$(EXEEXT) quartztest$(EXEEXT)

check_PROGRAMS = btreetest quartztest quartzdump quartzcheck quartzcompact

#FIXME: need to make these *after* libxapian, but want to install them...
#bin_PROGRAMS = quartzcheck quartzcompact

clean-local:
	rm -rf .quartztmp .btreetmp

btreetest_SOURCES = btreetest.cc btreecheck.cc
btreetest_LDADD = ../../testsuite/libtest.la ../../libxapian.la

quartztest_SOURCES = quartztest.cc
quartztest_LDADD = ../../getopt/libgetopt.la ../../testsuite/libtest.la \
		   ../../libxapian.la

quartzdump_SOURCES = quartzdump.cc
quartzdump_LDADD = ../../getopt/libgetopt.la ../../libxapian.la

quartzcheck_SOURCES = quartzcheck.cc btreecheck.cc
quartzcheck_LDADD = ../../libxapian.la

quartzcompact_SOURCES = quartzcompact.cc
quartzcompact_LDADD = ../../libxapian.la 
