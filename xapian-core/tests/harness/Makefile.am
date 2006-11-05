## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

#EXTRA_LTLIBRARIES = libbtreecheck.la

if BUILD_BACKEND_QUARTZ
backend_quartz_ltlibraries = libbtreecheck.la
endif

noinst_LTLIBRARIES = libharness.la $(backend_quartz_ltlibraries)

noinst_HEADERS = backendmanager.h testsuite.h testutils.h index_utils.h \
	btreecheck.h

libharness_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc index_utils.cc
libharness_la_LIBADD = ../../getopt/libgetopt.la

libbtreecheck_la_SOURCES = btreecheck.cc
libbtreecheck_la_LDFLAGS = ../../libxapian.la
