## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_builddir)/include

noinst_HEADERS =       flint_types.h \
                       flint_database.h \
		       flint_termlist.h \
		       flint_postlist.h \
		       flint_modifiedpostlist.h \
		       flint_positionlist.h \
		       flint_record.h \
		       flint_values.h \
		       flint_utils.h \
		       flint_document.h \
		       flint_alltermslist.h \
		       flint_table.h \
		       flint_cursor.h \
		       flint_btreeutil.h \
		       flint_btreebase.h \
		       flint_version.h \
		       flint_io.h \
		       flint_lock.h

libflint_la_SOURCES =  flint_database.cc \
		       flint_termlist.cc \
		       flint_postlist.cc \
		       flint_modifiedpostlist.cc \
		       flint_positionlist.cc \
		       flint_record.cc \
		       flint_values.cc \
		       flint_document.cc \
		       flint_alltermslist.cc \
		       flint_table.cc \
		       flint_cursor.cc \
		       flint_btreebase.cc \
		       flint_version.cc \
		       flint_io.cc \
		       flint_lock.cc

noinst_LTLIBRARIES = libflint.la
