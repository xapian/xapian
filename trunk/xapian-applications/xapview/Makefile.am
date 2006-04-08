## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 foreign # don't fuss about GNU required files

noinst_HEADERS = xapview.glade

AM_CXXFLAGS = $(XAPIAN_CFLAGS) $(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = xapview

xapview_SOURCES = xapview.cpp
xapview_LDADD = $(XAPIAN_LIBS) $(GTK_LIBS) $(LIBGLADE_LIBS)
