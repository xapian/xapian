## Process this file with automake to produce Makefile.in

noinst_HEADERS = querygui.glade querygui.h

INCLUDES = $(LIBOMUS_CFLAGS) $(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = querygui

querygui_SOURCES = querygui.cc
querygui_LDADD = @LIBOMUS_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@
