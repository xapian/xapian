bin_PROGRAMS = cvsmap

LIBDIRS = process range diff  aligned_diff cvs_parser map_algorithm html_writer xmlxx alignment
SUBDIRS = $(LIBDIRS) include

INCLUDES = $(foreach dir, $(LIBDIRS), -I$(srcdir)/$(dir)) -I$(srcdir)/../db -I$(srcdir)/include

cvsmap_SOURCES = map.cpp parse_change_log.h parse_change_log.cpp
cvsmap_LDADD = $(foreach dir, $(LIBDIRS), $(dir)/lib$(dir).a)  ../db/libcvs_db.a \
  -lm -lxml2 -lz -ldb_cxx $(foreach dir, $(LIBDIRS), $(dir)/lib$(dir).a) ../db/libcvs_db.a
cvsmap_LDFLAGS = -static
