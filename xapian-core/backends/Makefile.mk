EXTRA_DIST +=\
	backends/dir_contents\
	backends/Makefile

libxapian_la_SOURCES +=\
	backends/alltermslist.cc\
	backends/database.cc

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

# Define backend libraries to include.  To add a new one:
# i)   Add lines to configure.ac to define the automake conditional
#      "BUILD_BACKEND_NEWONE"
# ii)  Add lines below to "include backends/newone/Makefile.mk"
# iii) Write backends/newone/Makefile.mk - it should add files to
#      noinst_HEADERS and libxapian_la_SOURCES conditional on
#      BUILD_BACKEND_NEWONE.
# iv)  Write the backend code!
# v)   Update backends/database.cc.

include backends/chert/Makefile.mk
include backends/flint/Makefile.mk
include backends/inmemory/Makefile.mk
include backends/multi/Makefile.mk
include backends/remote/Makefile.mk
