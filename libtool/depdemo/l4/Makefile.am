## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies foreign

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libl4.la
libl4_la_SOURCES = l4.c l4.h
libl4_la_LDFLAGS = -no-undefined
libl4_la_LIBADD = $(top_builddir)/l3/libl3.la $(LIBM)
