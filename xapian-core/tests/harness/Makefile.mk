EXTRA_DIST +=\
	harness/dir_contents\
	harness/Makefile

noinst_HEADERS +=\
	harness/backendmanager.h\
	harness/index_utils.h\
	harness/rmdir.h\
	harness/testsuite.h\
	harness/testutils.h

testharness_sources =\
	harness/backendmanager.cc\
	harness/index_utils.cc\
	harness/rmdir.cc\
	harness/testsuite.cc\
	harness/testutils.cc
