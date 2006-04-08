bin_PROGRAMS = alignment
noinst_LTLIBRARIES = libalignment.la
INCDIRS = process diff range
INCLUDES = -I$(srcdir)/../process -I$(srcdir)/../diff -I$(srcdir)/../range\
    -I$(srcdir)/../include
alignment_SOURCES = alignment.cpp
alignment_LDADD = ../process/libprocess.la ../diff/libdiff.la \
    ../range/librange.la -lm libalignment.la

libalignment_la_SOURCES = char_sequence.cpp  line_sequence.cpp \
	alignment.h  alignment.hpp  char_sequence.h  line_sequence.h  sequence.h
AM_CXXFLAGS = -O3 -DNDEBUG -Wall -W
