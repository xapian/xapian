## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = $(LIBOMUS_CFLAGS) $(GTK_CFLAGS) $(LIBGLADE_CFLAGS)

bin_PROGRAMS = msearch

msearch_SOURCES = msearch.cc
msearch_LDADD = @LIBOMUS_LIBS@
