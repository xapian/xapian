## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = api.h header.h snowball_danish.h snowball_dutch.h\
 snowball_english.h snowball_finnish.h snowball_french.h snowball_german.h\
 snowball_italian.h snowball_lovins.h snowball_norwegian.h snowball_porter.h\
 snowball_portuguese.h snowball_russian.h snowball_spanish.h snowball_swedish.h

noinst_LTLIBRARIES = liblanguages.la

# snowball_*.c are machine generated sources which sometimes contain unused
# labels and variables
AM_CFLAGS = # FIXME: these are GCC specific: -Wall -W -Wno-unused

liblanguages_la_SOURCES = api.c snowball_danish.c snowball_dutch.c\
 snowball_english.c snowball_finnish.c snowball_french.c snowball_german.c\
 snowball_italian.c snowball_lovins.c snowball_norwegian.c snowball_porter.c\
 snowball_portuguese.c snowball_russian.c snowball_spanish.c snowball_swedish.c\
 utilities.c 
