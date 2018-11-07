EXTRA_DIST +=\
	backends/glass/Makefile

if BUILD_BACKEND_GLASS
noinst_HEADERS +=\
	backends/glass/glass_alldocspostlist.h\
	backends/glass/glass_alltermslist.h\
	backends/glass/glass_changes.h\
	backends/glass/glass_check.h\
	backends/glass/glass_cursor.h\
	backends/glass/glass_database.h\
	backends/glass/glass_databasereplicator.h\
	backends/glass/glass_dbcheck.h\
	backends/glass/glass_defs.h\
	backends/glass/glass_docdata.h\
	backends/glass/glass_document.h\
	backends/glass/glass_freelist.h\
	backends/glass/glass_inverter.h\
	backends/glass/glass_lazytable.h\
	backends/glass/glass_metadata.h\
	backends/glass/glass_positionlist.h\
	backends/glass/glass_postlist.h\
	backends/glass/glass_replicate_internal.h\
	backends/glass/glass_spelling.h\
	backends/glass/glass_spellingwordslist.h\
	backends/glass/glass_synonym.h\
	backends/glass/glass_table.h\
	backends/glass/glass_termlist.h\
	backends/glass/glass_termlisttable.h\
	backends/glass/glass_valuelist.h\
	backends/glass/glass_values.h\
	backends/glass/glass_version.h

lib_src +=\
	backends/glass/glass_alldocspostlist.cc\
	backends/glass/glass_alltermslist.cc\
	backends/glass/glass_changes.cc\
	backends/glass/glass_check.cc\
	backends/glass/glass_compact.cc\
	backends/glass/glass_cursor.cc\
	backends/glass/glass_database.cc\
	backends/glass/glass_dbcheck.cc\
	backends/glass/glass_document.cc\
	backends/glass/glass_freelist.cc\
	backends/glass/glass_inverter.cc\
	backends/glass/glass_metadata.cc\
	backends/glass/glass_positionlist.cc\
	backends/glass/glass_postlist.cc\
	backends/glass/glass_spelling.cc\
	backends/glass/glass_spellingwordslist.cc\
	backends/glass/glass_synonym.cc\
	backends/glass/glass_table.cc\
	backends/glass/glass_termlist.cc\
	backends/glass/glass_termlisttable.cc\
	backends/glass/glass_valuelist.cc\
	backends/glass/glass_values.cc\
	backends/glass/glass_version.cc

if BUILD_BACKEND_REMOTE
lib_src += \
	backends/glass/glass_databasereplicator.cc
endif

endif
