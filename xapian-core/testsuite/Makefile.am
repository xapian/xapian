## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_LTLIBRARIES = libtest.la

noinst_HEADERS = backendmanager.h

libtest_la_SOURCES = testsuite.cc testutils.cc backendmanager.cc
