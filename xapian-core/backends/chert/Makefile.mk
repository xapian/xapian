EXTRA_DIST +=\
	backends/chert/dir_contents\
	backends/chert/Makefile

if BUILD_BACKEND_CHERT
noinst_HEADERS +=\
	backends/chert/chert_alldocsmodifiedpostlist.h\
	backends/chert/chert_alldocspostlist.h\
	backends/chert/chert_alltermslist.h\
	backends/chert/chert_btreebase.h\
	backends/chert/chert_check.h\
	backends/chert/chert_compact.h\
	backends/chert/chert_cursor.h\
	backends/chert/chert_database.h\
	backends/chert/chert_databasereplicator.h\
	backends/chert/chert_dbstats.h\
	backends/chert/chert_document.h\
	backends/chert/chert_lazytable.h\
	backends/chert/chert_metadata.h\
	backends/chert/chert_modifiedpostlist.h\
	backends/chert/chert_positionlist.h\
	backends/chert/chert_postlist.h\
	backends/chert/chert_record.h\
	backends/chert/chert_replicate_internal.h\
	backends/chert/chert_spelling.h\
	backends/chert/chert_spellingwordslist.h\
	backends/chert/chert_synonym.h\
	backends/chert/chert_table.h\
	backends/chert/chert_termlist.h\
	backends/chert/chert_termlisttable.h\
	backends/chert/chert_types.h\
	backends/chert/chert_valuelist.h\
	backends/chert/chert_values.h\
	backends/chert/chert_version.h

lib_src +=\
	backends/chert/chert_alldocsmodifiedpostlist.cc\
	backends/chert/chert_alldocspostlist.cc\
	backends/chert/chert_alltermslist.cc\
	backends/chert/chert_btreebase.cc\
	backends/chert/chert_compact.cc\
	backends/chert/chert_cursor.cc\
	backends/chert/chert_database.cc\
	backends/chert/chert_databasereplicator.cc\
	backends/chert/chert_dbstats.cc\
	backends/chert/chert_document.cc\
	backends/chert/chert_metadata.cc\
	backends/chert/chert_modifiedpostlist.cc\
	backends/chert/chert_positionlist.cc\
	backends/chert/chert_postlist.cc\
	backends/chert/chert_record.cc\
	backends/chert/chert_spelling.cc\
	backends/chert/chert_spellingwordslist.cc\
	backends/chert/chert_synonym.cc\
	backends/chert/chert_table.cc\
	backends/chert/chert_termlist.cc\
	backends/chert/chert_termlisttable.cc\
	backends/chert/chert_valuelist.cc\
	backends/chert/chert_values.cc\
	backends/chert/chert_version.cc

noinst_LTLIBRARIES += libchertcheck.la

libchertcheck_la_SOURCES =\
	backends/chert/chert_check.cc
endif
