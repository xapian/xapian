## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = querygui.glade querygui.h

INCLUDES = -I$(top_srcdir)/../om/common -I$(top_srcdir)/../om/include \
	$(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = querygui

querygui_SOURCES = querygui.cc
querygui_LDADD = ../../../om/libom.la @GTK_LIBS@ @LIBGLADE_LIBS@
