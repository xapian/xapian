EXTRA_DIST +=\
	backends/dir_contents\
	backends/Makefile

libxapian_la_SOURCES +=\
	backends/database.cc
if BUILD_BACKEND_REMOTE
libxapian_la_SOURCES +=\
	backends/dbfactory_remote.cc
endif

# Define backend libraries to include.  To add a new one:
# i)   Add lines to configure.ac to define the automake conditional
#      "BUILD_BACKEND_NEWONE"
# ii)  Add lines below to "include newone/Makefile.mk"
# iii) Write newone/Makefile.mk - it should add files to noinst_HEADERS
#      and libxapian_la_SOURCES conditional on BUILD_BACKEND_NEWONE.
# iii) Write the backend code!

include backends/flint/Makefile.mk
include backends/inmemory/Makefile.mk
include backends/multi/Makefile.mk
include backends/quartz/Makefile.mk
include backends/remote/Makefile.mk
