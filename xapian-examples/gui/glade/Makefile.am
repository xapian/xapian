## Process this file with automake to produce Makefile.in

noinst_HEADERS = querygui.glade querygui.h

INCLUDES = $(XAPIAN_CFLAGS) $(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = querygui

querygui_SOURCES = querygui.cc
querygui_LDADD = @XAPIAN_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@
