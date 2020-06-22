EXTRA_DIST +=\
	backends/honey/Makefile

if BUILD_BACKEND_HONEY
noinst_HEADERS +=\
	backends/honey/honey_alldocspostlist.h\
	backends/honey/honey_alltermslist.h\
	backends/honey/honey_check.h\
	backends/honey/honey_cursor.h\
	backends/honey/honey_database.h\
	backends/honey/honey_dbcheck.h\
	backends/honey/honey_defs.h\
	backends/honey/honey_docdata.h\
	backends/honey/honey_document.h\
	backends/honey/honey_freelist.h\
	backends/honey/honey_inverter.h\
	backends/honey/honey_lazytable.h\
	backends/honey/honey_metadata.h\
	backends/honey/honey_positionlist.h\
	backends/honey/honey_postlist.h\
	backends/honey/honey_postlist_encodings.h\
	backends/honey/honey_postlisttable.h\
	backends/honey/honey_spelling.h\
	backends/honey/honey_spellingwordslist.h\
	backends/honey/honey_synonym.h\
	backends/honey/honey_table.h\
	backends/honey/honey_termlist.h\
	backends/honey/honey_termlisttable.h\
	backends/honey/honey_valuelist.h\
	backends/honey/honey_values.h\
	backends/honey/honey_version.h

lib_src +=\
	backends/honey/honey_alldocspostlist.cc\
	backends/honey/honey_alltermslist.cc\
	backends/honey/honey_check.cc\
	backends/honey/honey_compact.cc\
	backends/honey/honey_cursor.cc\
	backends/honey/honey_database.cc\
	backends/honey/honey_dbcheck.cc\
	backends/honey/honey_document.cc\
	backends/honey/honey_freelist.cc\
	backends/honey/honey_inverter.cc\
	backends/honey/honey_metadata.cc\
	backends/honey/honey_positionlist.cc\
	backends/honey/honey_postlist.cc\
	backends/honey/honey_postlisttable.cc\
	backends/honey/honey_spelling.cc\
	backends/honey/honey_spellingwordslist.cc\
	backends/honey/honey_synonym.cc\
	backends/honey/honey_table.cc\
	backends/honey/honey_termlist.cc\
	backends/honey/honey_termlisttable.cc\
	backends/honey/honey_valuelist.cc\
	backends/honey/honey_values.cc\
	backends/honey/honey_version.cc

endif
