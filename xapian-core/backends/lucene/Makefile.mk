EXTRA_DIST +=\
	backends/lucene/dir_contents\
	backends/lucene/Makefile

if BUILD_BACKEND_LUCENE
noinst_HEADERS +=\
	backends/lucene/lucene_database.h\
	backends/lucene/bytestream.h\
	backends/lucene/lucene_segmentgentable.h\
	backends/lucene/lucene_segmenttable.h\
	backends/lucene/lucene_tiitable.h\
	backends/lucene/lucene_term.h\
	backends/lucene/lucene_termindex.h\
	backends/lucene/lucene_fnmtable.h\
	backends/lucene/lucene_tistable.h\
	backends/lucene/lucene_frqtable.h\
	backends/lucene/lucene_fdtxtable.h\
	backends/lucene/lucene_segdb.h\
	backends/lucene/lucene_document.h

lib_src +=\
	backends/lucene/lucene_database.cc\
	backends/lucene/bytestream.cc\
	backends/lucene/lucene_segmentgentable.cc\
	backends/lucene/lucene_segmenttable.cc\
	backends/lucene/lucene_tiitable.cc\
	backends/lucene/lucene_term.cc\
	backends/lucene/lucene_termindex.cc\
	backends/lucene/lucene_fnmtable.cc\
	backends/lucene/lucene_tistable.cc\
	backends/lucene/lucene_frqtable.cc\
	backends/lucene/lucene_fdtxtable.cc\
	backends/lucene/lucene_segdb.cc\
	backends/lucene/lucene_document.cc

endif
