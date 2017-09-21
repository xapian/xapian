EXTRA_DIST +=\
	backends/inmemory/Makefile

# InMemoryPositionList is also used by DocumentTermList and the remote backend.
noinst_HEADERS +=\
	backends/inmemory/inmemory_positionlist.h

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
# Xapian::Document uses DocumentTermList which uses InMemoryPositionList so we
# always need "inmemory_positionlist.cc".
lib_src +=\
	backends/inmemory/inmemory_positionlist.cc
endif
