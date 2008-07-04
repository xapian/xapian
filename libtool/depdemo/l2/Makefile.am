## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies foreign

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libl2.la
libl2_la_SOURCES = l2.c l2.h
libl2_la_LDFLAGS = -no-undefined
libl2_la_LIBADD = $(top_builddir)/l1/libl1.la
