## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = musmutex.h io_system.h 3point6.h \
		 da_document.h da_database.h \
		 db_document.h db_database.h \
		 daread.h dbread.h dbdefs.h

noinst_LTLIBRARIES = libmuscat36.la

libmuscat36_la_SOURCES = io_system.c 3point6.c \
			 da_database.cc da_document.cc daread.c \
			 db_database.cc db_document.cc dbread.c

EXTRA_DIST = spec
