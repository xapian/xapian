## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies foreign

INCLUDES = -I$(top_srcdir)

extradir = $(libdir)/extra

lib_LTLIBRARIES = libl3.la
libl3_la_SOURCES = l3.c l3.h
libl3_la_LDFLAGS = -no-undefined
libl3_la_LIBADD = $(top_builddir)/l1/libl1.la $(top_builddir)/l2/libl2.la
