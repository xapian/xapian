bin_PROGRAMS = 
lib_LIBRARIES = libmap_algorithm.a
INCLUDES = -I../ -I../range -I../diff -I../cvs_parser -I../aligned_diff -I../alignment -I../process -I../html_writer -I../../db
libmap_algorithm_a_SOURCES = \
	backward_line_map_algorithm.cpp  \
	forward_map_algorithm.cpp        \
	line_map.cpp                     \
	map_algorithm.cpp                \
	range_map_algorithm.cpp          \
	backward_map_algorithm.cpp       \
	forward_range_map_algorithm.cpp  \
	line_map_algorithm.cpp           \
	range_map.cpp					 \
	backward_line_map_algorithm.h  forward_map_algorithm.h        line_map.h            map_algorithm.h \
	backward_map_algorithm.h       forward_range_map_algorithm.h  line_map_algorithm.h  \
	range_end_less_than.h  		   range_map_algorithm.h \
	range_begin_less_than.h  range_map.h
CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall
