bin_PROGRAMS = alignment
lib_LIBRARIES = libalignment.a
INCLUDES = -I../ -I../process -I../diff -I../range
alignment_SOURCES = alignment.cpp
alignment_LDADD = ../process/libprocess.a ../diff/libdiff.a ../range/librange.a libalignment.a\
 -lm ../process/libprocess.a ../diff/libdiff.a ../range/librange.a libalignment.a

libalignment_a_SOURCES = char_sequence.cpp  line_sequence.cpp \
	alignment.h  alignment.hpp  char_sequence.h  line_sequence.h  sequence.h
CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall
