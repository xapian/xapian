bin_PROGRAMS = cvsmap

LIBDIRS = process range diff  aligned_diff cvs_parser map_algorithm html_writer xmlxx alignment
SUBDIRS = $(LIBDIRS) include

INCLUDES = -I$(srcdir)/process -I$(srcdir)/range -I$(srcdir)/diff \
    -I$(srcdir)/aligned_diff -I$(srcdir)/cvs_parser -I$(srcdir)/map_algorithm \
    -I$(srcdir)/html_writer -I$(srcdir)/xmlxx -I$(srcdir)/alignment \
    -I$(srcdir)/../db -I$(srcdir)/include

if DYNAMIC
FLAGS = 
else
FLAGS = -static
endif

cvsmap_SOURCES = map.cpp parse_change_log.h parse_change_log.cpp
cvsmap_LDADD = process/libprocess.la cvs_parser/libcvs_parser.la \
    map_algorithm/libmap_algorithm.la aligned_diff/libaligned_diff.la \
    range/librange.la diff/libdiff.la html_writer/libhtml_writer.la \
    xmlxx/libxmlxx.la alignment/libalignment.la ../db/libcvs_db.la \
    -lm -lxml2 -lz -ldb_cxx
cvsmap_LDFLAGS = $(FLAGS)
