## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS =

INCLUDES = $(LIBOMUS_CFLAGS)

bin_PROGRAMS = mgrep

mgrep_SOURCES = mgrep.cc
mgrep_LDADD = @LIBOMUS_LIBS@
