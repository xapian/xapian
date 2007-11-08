EXTRA_DIST +=\
	harness/dir_contents\
	harness/Makefile

noinst_HEADERS +=\
	harness/backendmanager.h\
	harness/backendmanager_flint.h\
	harness/backendmanager_inmemory.h\
	harness/backendmanager_quartz.h\
	harness/backendmanager_local.h\
	harness/backendmanager_multi.h\
	harness/backendmanager_remoteprog.h\
	harness/backendmanager_remotetcp.h\
	harness/index_utils.h\
	harness/unixcmds.h\
	harness/testsuite.h\
	harness/testutils.h

testharness_sources =\
	harness/backendmanager.cc\
	harness/backendmanager_multi.cc\
	harness/index_utils.cc\
	harness/testsuite.cc\
	harness/testutils.cc\
	harness/unixcmds.cc

if BUILD_BACKEND_FLINT
testharness_sources += harness/backendmanager_flint.cc
endif

if BUILD_BACKEND_INMEMORY
testharness_sources += harness/backendmanager_inmemory.cc
endif

if BUILD_BACKEND_QUARTZ
testharness_sources += harness/backendmanager_quartz.cc
endif

if BUILD_BACKEND_REMOTE
testharness_sources +=\
	harness/backendmanager_remoteprog.cc\
	harness/backendmanager_remotetcp.cc
endif
