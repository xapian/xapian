## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I${top_builddir}/include

noinst_LTLIBRARIES = libharness.la

noinst_HEADERS = backendmanager.h testsuite.h testutils.h index_utils.h

libharness_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc index_utils.cc
libharness_la_LIBADD = ../../getopt/libgetopt.la
