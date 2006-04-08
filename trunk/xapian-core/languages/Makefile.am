## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = api.h header.h snowball_danish.h snowball_dutch.h\
 snowball_english.h snowball_finnish.h snowball_french.h snowball_german.h\
 snowball_italian.h snowball_lovins.h snowball_norwegian.h snowball_porter.h\
 snowball_portuguese.h snowball_russian.h snowball_spanish.h snowball_swedish.h

noinst_LTLIBRARIES = liblanguages.la

# snowball_*.cc are machine generated sources which sometimes contain unused
# labels and variables, so turn off warnings.
AM_CXXFLAGS =

liblanguages_la_SOURCES = api.cc snowball_danish.cc snowball_dutch.cc\
 snowball_english.cc snowball_finnish.cc snowball_french.cc snowball_german.cc\
 snowball_italian.cc snowball_lovins.cc snowball_norwegian.cc\
 snowball_porter.cc snowball_portuguese.cc snowball_russian.cc\
 snowball_spanish.cc snowball_swedish.cc utilities.cc 
