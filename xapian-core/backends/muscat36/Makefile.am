## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = io_system.h 3point6.h \
		 da_document.h damuscat.h daread.h da_database.h

noinst_LTLIBRARIES = libmuscat36.la

COMMON_SRC = io_system.c 3point6.c

libmuscat36_la_SOURCES = $(COMMON_SRC) \
			 da_database.cc da_document.cc daread.c 
