EXTRA_DIST +=\
	backends/quartz/dir_contents\
	backends/quartz/Makefile

if BUILD_BACKEND_QUARTZ
noinst_HEADERS +=\
	backends/quartz/bcursor.h\
	backends/quartz/btree_base.h\
	backends/quartz/btreecheck.h\
	backends/quartz/btree.h\
	backends/quartz/btree_util.h\
	backends/quartz/quartz_alldocspostlist.h\
	backends/quartz/quartz_alltermslist.h\
	backends/quartz/quartz_database.h\
	backends/quartz/quartz_document.h\
	backends/quartz/quartz_log.h\
	backends/quartz/quartz_metafile.h\
	backends/quartz/quartz_positionlist.h\
	backends/quartz/quartz_postlist.h\
	backends/quartz/quartz_record.h\
	backends/quartz/quartz_termlist.h\
	backends/quartz/quartz_types.h\
	backends/quartz/quartz_utils.h\
	backends/quartz/quartz_values.h

libxapian_la_SOURCES +=\
	backends/quartz/bcursor.cc\
	backends/quartz/btree_base.cc\
	backends/quartz/btree.cc\
	backends/quartz/quartz_alldocspostlist.cc\
	backends/quartz/quartz_alltermslist.cc\
	backends/quartz/quartz_database.cc\
	backends/quartz/quartz_document.cc\
	backends/quartz/quartz_log.cc\
	backends/quartz/quartz_metafile.cc\
	backends/quartz/quartz_positionlist.cc\
	backends/quartz/quartz_postlist.cc\
	backends/quartz/quartz_record.cc\
	backends/quartz/quartz_termlist.cc\
	backends/quartz/quartz_values.cc

noinst_LTLIBRARIES += backends/quartz/libbtreecheck.la

backends_quartz_libbtreecheck_la_SOURCES =\
	backends/quartz/btreecheck.cc
endif
