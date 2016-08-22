EXTRA_DIST +=\
	harness/Makefile

noinst_HEADERS +=\
	harness/backendmanager.h\
	harness/cputimer.h\
	harness/fdtracker.h\
	harness/index_utils.h\
	harness/unixcmds.h\
	harness/scalability.h\
	harness/testmacros.h\
	harness/testrunner.h\
	harness/testsuite.h\
	harness/testutils.h

testharness_sources =\
	harness/backendmanager.cc\
	harness/cputimer.cc\
	harness/fdtracker.cc\
	harness/index_utils.cc\
	harness/scalability.cc\
	harness/testrunner.cc\
	harness/testsuite.cc\
	harness/testutils.cc\
	harness/unixcmds.cc

# CYGWIN and MINGW lack std::to_string(), so we use str() which is private to
# the library, and then have to link its object directly.
testharness_sources += ../common/str.cc

utestharness_sources =\
	harness/fdtracker.cc\
	harness/utestsuite.cc
