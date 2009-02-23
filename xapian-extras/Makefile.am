## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.5 subdir-objects
ACLOCAL_AMFLAGS = -I m4

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

INCLUDES =
noinst_HEADERS =
EXTRA_DIST =
noinst_LTLIBRARIES =

lib_LTLIBRARIES = libxapian_imgseek.la
libxapian_imgseek_la_SOURCES =

include imgseek/Makefile.mk
