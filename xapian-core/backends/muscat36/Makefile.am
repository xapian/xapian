## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = io_system.h 3point6.h \
		 da_document.h da_database.h \
		 db_document.h db_database.h \
		 daread.h dbread.h dbdefs.h \
		 da_alltermslist.h

noinst_LTLIBRARIES = libmuscat36.la

libmuscat36_la_SOURCES = io_system.cc 3point6.cc \
			 da_database.cc da_document.cc daread.cc \
			 da_alltermslist.cc \
			 db_database.cc db_document.cc dbread.cc
