noinst_HEADERS = cvs_comment_db.h cvs_db.h cvs_file_revision_db.h \
 cvs_line_db.h cvs_comment_id_db.h cvs_db_file.h cvs_filename_db.h \
 cvs_revision_db.h cvs_comment_id2_db.h cvs_file_id_db.h cvs_revision_less.h \
 cvs_diff_db.h \
 util.h lines.h lines_cmt.h lines_db.h

bin_PROGRAMS = cvsindex cvssearch cvsquery cvsmineindex cvsminesearch \
 cvscommitindex cvscommitsearch cvsfileindex cvsfilesearch cvsrelatedindex \
 cvsrelatedsearch

noinst_LTLIBRARIES = libcvs_db.la libcvs_lines.la

libcvs_db_la_SOURCES = cvs_comment_db.C cvs_db.C cvs_file_revision_db.C \
 cvs_line_db.C cvs_comment_id_db.C cvs_db_file.C cvs_filename_db.C \
 cvs_revision_db.C cvs_diff_db.C cvs_comment_id2_db.C cvs_file_id_db.C

libcvs_lines_la_SOURCES = util.C lines.C lines_cmt.C lines_db.C

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS)
libs = $(noinst_LTLIBRARIES) $(XAPIAN_LIBS) -ldb_cxx

if DYNAMIC
FLAGS =
else
FLAGS = -static
endif

cvsquery_SOURCES = cvsquery.C
cvsquery_LDFLAGS = $(FLAGS)
cvsquery_LDADD = $(libs)

cvsindex_SOURCES = cvsindex.C
cvsindex_LDFLAGS = $(FLAGS)
cvsindex_LDADD = $(libs)

cvssearch_SOURCES = cvssearch.C
cvssearch_LDFLAGS = $(FLAGS)
cvssearch_LDADD = $(libs)


cvsminesearch_SOURCES = cvsminesearch.C
cvsminesearch_LDFLAGS = $(FLAGS)
cvsminesearch_LDADD = $(libs)

cvsmineindex_SOURCES = cvsmineindex.C
cvsmineindex_LDADD = $(libs)
cvsmineindex_LDFLAGS = $(FLAGS)

cvscommitsearch_SOURCES = cvscommitsearch.C
cvscommitsearch_LDFLAGS = $(FLAGS)
cvscommitsearch_LDADD = $(libs)

cvscommitindex_SOURCES = cvscommitindex.C
cvscommitindex_LDADD = $(libs)
cvscommitindex_LDFLAGS = $(FLAGS)


cvsfilesearch_SOURCES = cvsfilesearch.C
cvsfilesearch_LDFLAGS = $(FLAGS)
cvsfilesearch_LDADD = $(libs)

cvsfileindex_SOURCES = cvsfileindex.C
cvsfileindex_LDADD = $(libs)
cvsfileindex_LDFLAGS = $(FLAGS)

cvsrelatedsearch_SOURCES = cvsrelatedsearch.C
cvsrelatedsearch_LDFLAGS = $(FLAGS)
cvsrelatedsearch_LDADD = $(libs)

cvsrelatedindex_SOURCES = cvsrelatedindex.C
cvsrelatedindex_LDADD = $(libs)
cvsrelatedindex_LDFLAGS = $(FLAGS)
