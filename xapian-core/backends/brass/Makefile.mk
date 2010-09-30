EXTRA_DIST +=\
	backends/brass/dir_contents\
	backends/brass/Makefile

if BUILD_BACKEND_BRASS
noinst_HEADERS +=\
	backends/brass/brass_alldocspostlist.h\
	backends/brass/brass_alltermslist.h\
	backends/brass/brass_btreebase.h\
	backends/brass/brass_check.h\
	backends/brass/brass_compact.h\
	backends/brass/brass_cursor.h\
	backends/brass/brass_database.h\
	backends/brass/brass_databasereplicator.h\
	backends/brass/brass_dbstats.h\
	backends/brass/brass_document.h\
	backends/brass/brass_inverter.h\
	backends/brass/brass_lazytable.h\
	backends/brass/brass_metadata.h\
	backends/brass/brass_positionlist.h\
	backends/brass/brass_postlist.h\
	backends/brass/brass_record.h\
	backends/brass/brass_replicate_internal.h\
	backends/brass/brass_spelling.h\
	backends/brass/brass_spellingwordslist.h\
	backends/brass/brass_synonym.h\
	backends/brass/brass_table.h\
	backends/brass/brass_termlist.h\
	backends/brass/brass_termlisttable.h\
	backends/brass/brass_types.h\
	backends/brass/brass_valuelist.h\
	backends/brass/brass_values.h\
	backends/brass/brass_version.h

lib_src +=\
	backends/brass/brass_alldocspostlist.cc\
	backends/brass/brass_alltermslist.cc\
	backends/brass/brass_btreebase.cc\
	backends/brass/brass_compact.cc\
	backends/brass/brass_cursor.cc\
	backends/brass/brass_database.cc\
	backends/brass/brass_databasereplicator.cc\
	backends/brass/brass_dbstats.cc\
	backends/brass/brass_document.cc\
	backends/brass/brass_inverter.cc\
	backends/brass/brass_metadata.cc\
	backends/brass/brass_positionlist.cc\
	backends/brass/brass_postlist.cc\
	backends/brass/brass_record.cc\
	backends/brass/brass_spelling.cc\
	backends/brass/brass_spellingwordslist.cc\
	backends/brass/brass_synonym.cc\
	backends/brass/brass_table.cc\
	backends/brass/brass_termlist.cc\
	backends/brass/brass_termlisttable.cc\
	backends/brass/brass_valuelist.cc\
	backends/brass/brass_values.cc\
	backends/brass/brass_version.cc

noinst_LTLIBRARIES += libbrasscheck.la

libbrasscheck_la_SOURCES =\
	backends/brass/brass_check.cc
endif
