## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.4.2.1 2006/02/03 09:37:34 rwild Exp $

AUTOMAKE_OPTIONS = no-dependencies foreign

EXTRA_DIST = acinclude.m4

noinst_LTLIBRARIES = libfoo.la
lib_LTLIBRARIES = libbaz.la

libfoo_la_SOURCES = foo.cpp
libfoo_la_LDFLAGS = -no-undefined
libfoo_la_LIBADD = $(LIBM)

# Test some of the ILD support when using tagged configurations.
libbaz_la_SOURCES = baz.cpp
libbaz_la_LDFLAGS = -no-undefined
libbaz_la_LIBADD = libfoo.la

noinst_HEADERS = foo.h baz.h

bin_PROGRAMS = tagdemo

tagdemo_SOURCES = main.cpp
tagdemo_LDADD = libbaz.la

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
