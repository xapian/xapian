noinst_LIBRARIES = libcvs_db.a libcvs_lines.a
libcvs_db_a_SOURCES = cvs_comment_db.C     cvs_db.C       cvs_file_revision_db.C  cvs_line_db.C \
 cvs_comment_db.h     cvs_db.h       cvs_file_revision_db.h  cvs_line_db.h \
 cvs_comment_id_db.C  cvs_db_file.C  cvs_filename_db.C       cvs_revision_db.C \
 cvs_comment_id_db.h  cvs_db_file.h  cvs_filename_db.h       cvs_revision_db.h \
 cvs_comment_id2_db.h cvs_comment_id2_db.C \
 cvs_file_id_db.h cvs_file_id_db.C \
 cvs_revision_less.h \
 cvs_diff_db.C cvs_diff_db.h

libcvs_lines_a_SOURCES = util.C util.h lines.h lines.C lines_cmt.C lines_cmt.h lines_db.C lines_db.h

bin_PROGRAMS = cvsindex cvssearch cvsquery cvsmineindex cvsminesearch cvscommitindex cvscommitsearch cvsfileindex cvsfilesearch cvsrelatedindex cvsrelatedsearch

LIBS = $(noinst_LIBRARIES) -lxapian -ldb_cxx

if DYNAMIC
FLAGS =
else
FLAGS = -static
endif

cvsquery_SOURCES = cvsquery.C
cvsquery_LDFLAGS = $(FLAGS)
cvsquery_LDADD = $(LIBS)

cvsindex_SOURCES = cvsindex.C
cvsindex_LDFLAGS = $(FLAGS)
cvsindex_LDADD = $(LIBS)

cvssearch_SOURCES = cvssearch.C
cvssearch_LDFLAGS = $(FLAGS)
cvssearch_LDADD = $(LIBS)


cvsminesearch_SOURCES =  cvsminesearch.C
cvsminesearch_LDFLAGS = $(FLAGS)
cvsminesearch_LDADD = $(LIBS)

cvsmineindex_SOURCES =   cvsmineindex.C
cvsmineindex_LDADD = $(LIBS)
cvsmineindex_LDFLAGS = $(FLAGS)

cvscommitsearch_SOURCES =  cvscommitsearch.C
cvscommitsearch_LDFLAGS = $(FLAGS)
cvscommitsearch_LDADD = $(LIBS)

cvscommitindex_SOURCES =   cvscommitindex.C
cvscommitindex_LDADD = $(LIBS)
cvscommitindex_LDFLAGS = $(FLAGS)


cvsfilesearch_SOURCES =  cvsfilesearch.C
cvsfilesearch_LDFLAGS = $(FLAGS)
cvsfilesearch_LDADD = $(LIBS)

cvsfileindex_SOURCES =   cvsfileindex.C
cvsfileindex_LDADD = $(LIBS)
cvsfileindex_LDFLAGS = $(FLAGS)

cvsrelatedsearch_SOURCES =  cvsrelatedsearch.C
cvsrelatedsearch_LDFLAGS = $(FLAGS)
cvsrelatedsearch_LDADD = $(LIBS)

cvsrelatedindex_SOURCES =   cvsrelatedindex.C
cvsrelatedindex_LDADD = $(LIBS)
cvsrelatedindex_LDFLAGS = $(FLAGS)
