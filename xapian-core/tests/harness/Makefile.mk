EXTRA_DIST +=\
	harness/dir_contents

noinst_HEADERS +=\
	harness/backendmanager.h\
	harness/index_utils.h\
	harness/testsuite.h\
	harness/testutils.h

testharness_sources =\
	harness/backendmanager.cc\
	harness/index_utils.cc\
	harness/testsuite.cc\
	harness/testutils.cc
