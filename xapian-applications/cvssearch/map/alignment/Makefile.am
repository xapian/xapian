bin_PROGRAMS = alignment
noinst_LIBRARIES = libalignment.a
INCDIRS = process diff range
INCLUDES = $(foreach dir, $(INCDIRS), -I$(srcdir)/../$(dir)) -I$(srcdir)/../include
alignment_SOURCES = alignment.cpp
alignment_LDADD = $(foreach dir, $(INCDIRS), ../$(dir)/lib$(dir).a) libalignment.a \
              -lm $(foreach dir, $(INCDIRS), ../$(dir)/lib$(dir).a) libalignment.a

libalignment_a_SOURCES = char_sequence.cpp  line_sequence.cpp \
	alignment.h  alignment.hpp  char_sequence.h  line_sequence.h  sequence.h
CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall
