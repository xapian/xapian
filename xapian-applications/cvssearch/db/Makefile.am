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

bin_PROGRAMS = cvsindex cvssearch cvsquery cvsmineindex cvsmine2index cvsminesearch cvsmine2search cvsfileindex cvsfilesearch \
	# cvssearch cvsusagesearch cvscomments \
	# cvssymbolcount cvscountsymbols cvsminelocal cvsquery #cvsmineglobal#cvseval

LIBS = $(noinst_LIBRARIES) -lomseek -ldb_cxx

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

cvsmine2search_SOURCES =  cvsmine2search.C
cvsmine2search_LDFLAGS = $(FLAGS)
cvsmine2search_LDADD = $(LIBS)

cvsfilesearch_SOURCES =   cvsfilesearch.C
cvsfilesearch_LDFLAGS = $(FLAGS)
cvsfilesearch_LDADD = $(LIBS) 

#cvsusagesearch_SOURCES =   cvsusagesearch.C
#cvsusagesearch_LDFLAGS = $(FLAGS) 
#cvsusagesearch_LDADD = $(LIBS) 

#cvsclasses_SOURCES =   cvspatterns.C
#cvsclasses_LDADD = $(LIBS) 

#cvsfunctions_SOURCES =   cvspatterns.C
#cvsfunctions_LDADD = $(LIBS) 

#cvsusageindex_SOURCES =   cvsusageindex.C
#cvsusageindex_LDADD = $(LIBS) 
#cvsusageindex_LDFLAGS = $(FLAGS)

cvsmineindex_SOURCES =   cvsmineindex.C
cvsmineindex_LDADD = $(LIBS) 
cvsmineindex_LDFLAGS = $(FLAGS)

cvsmine2index_SOURCES =   cvsmine2index.C
cvsmine2index_LDADD = $(LIBS)
cvsmine2index_LDFLAGS = $(FLAGS)

cvsfileindex_SOURCES =   cvsfileindex.C
cvsfileindex_LDADD = $(LIBS)
cvsfileindex_LDFLAGS = $(FLAGS)

#cvscomments_SOURCES = cvscomments.C
#cvscomments_LDADD = $(LIBS)
#cvscomments_LDFLAGS = $(FLAGS)

#cvssymbolcount_SOURCES = cvssymbolcount.C
#cvssymbolcount_LDADD = $(LIBS)
#cvssymbolcount_LDFLAGS = $(FLAGS)

#cvscountsymbols_SOURCES =   cvscountsymbols.C
#cvscountsymbols_LDADD = $(LIBS)
#cvscountsymbols_LDFLAGS = $(FLAGS)

#cvsminelocal_SOURCES =   cvsminelocal.C
#cvsminelocal_LDADD = $(LIBS)
#cvsminelocal_LDFLAGS = $(FLAGS)

#cvsmineglobal_SOURCES =   cvsmineglobal.C
#cvsmineglobal_LDADD = $(LIBS)

#cvseval_SOURCES =   cvseval.C

#cvscomments_SOURCES = cvscomments.C
#cvscomments_LDADD = $(LIBS)

#cvssymbolcount_SOURCES = cvssymbolcount.C
#cvssymbolcount_LDADD = $(LIBS)

#cvscountsymbols_SOURCES =   cvscountsymbols.C
#cvscountsymbols_LDADD = $(LIBS)

#cvsminelocal_SOURCES =   cvsminelocal.C
#cvsminelocal_LDADD = $(LIBS) 

#cvsmineglobal_SOURCES =   cvsmineglobal.C
#cvsmineglobal_LDADD = $(LIBS)


#cvseval_SOURCES =   cvseval.C
#cvseval_LDADD = $(LIBS)
