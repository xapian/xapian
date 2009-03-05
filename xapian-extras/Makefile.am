## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 subdir-objects
ACLOCAL_AMFLAGS = -I m4

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) $(QT_CFLAGS)
LIBS = $(XAPIAN_LIBS) $(QT_LIBS)

INCLUDES =
noinst_HEADERS =
EXTRA_DIST =
noinst_LTLIBRARIES =

lib_LTLIBRARIES = libxapian_imgseek.la
libxapian_imgseek_la_SOURCES =

xapianincludedir = $(includedir)/xapian
xapianinclude_HEADERS =

include imgseek/Makefile.mk
