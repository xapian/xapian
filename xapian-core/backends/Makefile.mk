EXTRA_DIST +=\
	backends/dir_contents\
	backends/Makefile

libxapian_la_SOURCES +=\
	backends/alltermslist.cc\
	backends/database.cc\
	backends/dbfactory.cc\
	backends/valuelist.cc

if BUILD_BACKEND_REMOTE
libxapian_la_SOURCES +=\
	backends/dbfactory_remote.cc
endif

if BUILD_BACKEND_FLINT
libxapian_la_SOURCES +=\
	backends/contiguousalldocspostlist.cc
else
if BUILD_BACKEND_CHERT
libxapian_la_SOURCES +=\
        backends/contiguousalldocspostlist.cc
endif
endif

# To add a new database backend:
#
# 1) Add lines to configure.ac to define the automake conditional
#    "BUILD_BACKEND_NEWONE"
# 2) Add lines below to "include backends/newone/Makefile.mk"
# 3) Write backends/newone/Makefile.mk - it should add files to noinst_HEADERS
#    and libxapian_la_SOURCES conditional on BUILD_BACKEND_NEWONE.
# 4) Update backends/dbfactory.cc.
# 5) Write the backend code!

include backends/chert/Makefile.mk
include backends/flint/Makefile.mk
include backends/inmemory/Makefile.mk
include backends/multi/Makefile.mk
include backends/remote/Makefile.mk
