EXTRA_DIST +=\
	backends/inmemory/dir_contents\
	backends/inmemory/Makefile

if BUILD_BACKEND_INMEMORY
noinst_HEADERS +=\
	backends/inmemory/inmemory_alltermslist.h\
	backends/inmemory/inmemory_database.h\
	backends/inmemory/inmemory_document.h

lib_src +=\
	backends/inmemory/inmemory_alltermslist.cc\
	backends/inmemory/inmemory_database.cc\
	backends/inmemory/inmemory_document.cc\
	backends/inmemory/inmemory_positionlist.cc
else
# Xapian::Document uses MapTermList which uses InMemoryPositionList so we
# always need "inmemory_positionlist.cc".
lib_src +=\
	backends/inmemory/inmemory_positionlist.cc
endif
