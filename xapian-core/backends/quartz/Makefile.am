## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/testsuite

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
		       quartz_attributes.h \
		       quartz_utils.h \
		       quartz_log.h \
		       quartz_document.h \
		       quartz_alltermslist.h \
		       quartz_metafile.h \
		       btree.h \
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
		       quartz_attributes.cc \
		       quartz_utils.cc \
		       quartz_log.cc \
		       quartz_document.cc \
		       quartz_alltermslist.cc \
		       quartz_metafile.cc \
		       btree.cc \
		       bcursor.cc \
		       btree_base.cc


noinst_LTLIBRARIES = libquartz.la

TESTS = run-quartztest run-btreetest

check_PROGRAMS = btreetest \
		 quartztest \
                 quartzdump

quartztest_SOURCES = quartztest.cc
quartztest_LDADD = libquartz.la \
                   ../../testsuite/libtest.la \
		   ../../libomsee.la

btreetest_SOURCES = btreetest.cc btree.cc bcursor.cc btree_base.cc
btreetest_LDADD = ../../testsuite/libtest.la \
		  ../../common/libcommon.la \
		  ../../api/omerror.lo

quartzdump_SOURCES = quartzdump.cc
quartzdump_LDADD = libquartz.la \
		   ../../libomsee.la

run-quartztest:
	@rm -f run-quartztest
	$(LN_S) $(top_srcdir)/testsuite/malloc_wrapper.sh run-quartztest

run-btreetest:
	@rm -f run-btreetest
	$(LN_S) $(top_srcdir)/testsuite/malloc_wrapper.sh run-btreetest

