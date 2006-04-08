bin_PROGRAMS = 
noinst_LTLIBRARIES = libmap_algorithm.la
INCDIRS = include range diff cvs_parser aligned_diff alignment process html_writer ../db
INCLUDES = $(foreach dir, $(INCDIRS), -I$(srcdir)/../$(dir))
libmap_algorithm_la_SOURCES = \
	backward_line_map_algorithm.cpp  \
	forward_map_algorithm.cpp        \
	line_map.cpp                     \
	map_algorithm.cpp                \
	range_map_algorithm.cpp          \
	backward_map_algorithm.cpp       \
	forward_range_map_algorithm.cpp  \
	line_map_algorithm.cpp           \
	range_map.cpp			 \
	backward_line_map_algorithm.h  forward_map_algorithm.h        line_map.h            map_algorithm.h \
	backward_map_algorithm.h       forward_range_map_algorithm.h  line_map_algorithm.h  \
	range_end_less_than.h  		   range_map_algorithm.h \
	range_begin_less_than.h  range_map.h
