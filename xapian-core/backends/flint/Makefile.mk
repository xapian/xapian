EXTRA_DIST +=\
	backends/flint/dir_contents\
	backends/flint/Makefile

if BUILD_BACKEND_FLINT
noinst_HEADERS +=\
	backends/flint/flint_alldocspostlist.h\
	backends/flint/flint_alltermslist.h\
	backends/flint/flint_btreebase.h\
	backends/flint/flint_btreeutil.h\
	backends/flint/flint_cursor.h\
	backends/flint/flint_database.h\
	backends/flint/flint_document.h\
	backends/flint/flint_io.h\
	backends/flint/flint_lock.h\
	backends/flint/flint_modifiedpostlist.h\
	backends/flint/flint_positionlist.h\
	backends/flint/flint_postlist.h\
	backends/flint/flint_record.h\
	backends/flint/flint_table.h\
	backends/flint/flint_termlist.h\
	backends/flint/flint_types.h\
	backends/flint/flint_utils.h\
	backends/flint/flint_values.h\
	backends/flint/flint_version.h

libxapian_la_SOURCES +=\
	backends/flint/flint_alldocspostlist.cc\
	backends/flint/flint_alltermslist.cc\
	backends/flint/flint_btreebase.cc\
	backends/flint/flint_cursor.cc\
	backends/flint/flint_database.cc\
	backends/flint/flint_document.cc\
	backends/flint/flint_io.cc\
	backends/flint/flint_lock.cc\
	backends/flint/flint_modifiedpostlist.cc\
	backends/flint/flint_positionlist.cc\
	backends/flint/flint_postlist.cc\
	backends/flint/flint_record.cc\
	backends/flint/flint_table.cc\
	backends/flint/flint_termlist.cc\
	backends/flint/flint_values.cc\
	backends/flint/flint_version.cc
endif
