## Process this file with automake to produce Makefile.in

noinst_HEADERS = xapview.glade

INCLUDES = $(XAPIAN_CFLAGS) $(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = xapview

xapview_SOURCES = xapview.cpp
xapview_LDADD = @XAPIAN_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@
