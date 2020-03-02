noinst_HEADERS +=\
	backends/alltermslist.h\
	backends/backends.h\
	backends/byte_length_strings.h\
	backends/contiguousalldocspostlist.h\
	backends/databasehelpers.h\
	backends/databaseinternal.h\
	backends/databasereplicator.h\
	backends/documentinternal.h\
	backends/empty_database.h\
	backends/flint_lock.h\
	backends/leafpostlist.h\
	backends/multi.h\
	backends/positionlist.h\
	backends/postlist.h\
	backends/prefix_compressed_strings.h\
	backends/slowvaluelist.h\
	backends/uuids.h\
	backends/valuelist.h\
	backends/valuestats.h

EXTRA_DIST +=\
	backends/Makefile

lib_src +=\
	backends/alltermslist.cc\
	backends/dbcheck.cc\
	backends/databasehelpers.cc\
	backends/databaseinternal.cc\
	backends/databasereplicator.cc\
	backends/dbfactory.cc\
	backends/documentinternal.cc\
	backends/empty_database.cc\
	backends/leafpostlist.cc\
	backends/postlist.cc\
	backends/slowvaluelist.cc\
	backends/uuids.cc\
	backends/valuelist.cc

if BUILD_BACKEND_REMOTE
lib_src +=\
	backends/dbfactory_remote.cc
endif

if BUILD_BACKEND_GLASS
lib_src +=\
	backends/contiguousalldocspostlist.cc\
	backends/flint_lock.cc
else
if BUILD_BACKEND_HONEY
lib_src +=\
	backends/contiguousalldocspostlist.cc\
	backends/flint_lock.cc
endif
endif

# To add a new database backend:
#
# 1) Add lines to configure.ac to define the automake conditional
#    "BUILD_BACKEND_NEWONE" and add NEWONE to the "for backend in" loop.
# 2) Update include/xapian/version_h.cc to handle XAPIAN_HAS_NEWONE_BACKEND.
# 3) Add "include backends/newone/Makefile.mk" to the list below.
# 4) Write backends/newone/Makefile.mk - it should add files to noinst_HEADERS
#    and lib_src conditional on BUILD_BACKEND_NEWONE.
# 5) Update backends/dbfactory.cc.
# 6) If it needs to support replication, update backends/databasereplicator.cc
# 7) Write the backend code!

include backends/glass/Makefile.mk
include backends/honey/Makefile.mk
include backends/inmemory/Makefile.mk
include backends/multi/Makefile.mk
include backends/remote/Makefile.mk
