## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = querygui.glade querygui.h

INCLUDES = $(LIBOMUS_CFLAGS) $(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = querygui

querygui_SOURCES = querygui.cc
querygui_LDADD = @LIBOMUS_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@
