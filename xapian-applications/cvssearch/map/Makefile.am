bin_PROGRAMS = cvsmap

LIBDIRS = process range diff  aligned_diff alignment cvs_parser map_algorithm html_writer
SUBDIRS = process range diff  aligned_diff cvs_parser map_algorithm html_writer include alignment

INCLUDES = $(foreach dir, $(LIBDIRS), -I$(srcdir)/$(dir)) -I$(srcdir)/../db -I$(srcdir)/include

cvsmap_SOURCES = map.cpp
cvsmap_LDADD = $(foreach dir, $(LIBDIRS), $(dir)/lib$(dir).a)  ../db/libcvs_db.a \
  -lm -ldb_cxx $(foreach dir, $(LIBDIRS), $(dir)/lib$(dir).a) ../db/libcvs_db.a

