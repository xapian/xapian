noinst_LIBRARIES = libcvs_db.a libcvs_lines.a
libcvs_db_a_SOURCES = cvs_comment_db.C     cvs_db.C       cvs_file_revision_db.C  cvs_line_db.C \
 cvs_comment_db.h     cvs_db.h       cvs_file_revision_db.h  cvs_line_db.h \
 cvs_comment_id_db.C  cvs_db_file.C  cvs_filename_db.C       cvs_revision_db.C \
 cvs_comment_id_db.h  cvs_db_file.h  cvs_filename_db.h       cvs_revision_db.h \
 cvs_comment_id2_db.h cvs_comment_id2_db.C \
 cvs_file_id_db.h cvs_file_id_db.C \
 cvs_revision_less.h \
 cvs_diff_db.C cvs_diff_db.h

libcvs_lines_a_SOURCES = util.C util.h lines.h lines.C lines_cmt.C lines_cmt.h

bin_PROGRAMS = cvsindex cvssearch cvsquery cvsmineindex cvsminesearch cvsfileindex cvsfilesearch \
	# cvssearch cvsusagesearch cvscomments \
	# cvssymbolcount cvscountsymbols cvsminelocal cvsquery #cvsmineglobal#cvseval

cvsquery_SOURCES = cvsquery.C
cvsquery_LDADD = libcvs_db.a -ldb_cxx libcvs_db.a

cvsindex_SOURCES = cvsindex.C
cvsindex_LDFLAGS = -static
cvsindex_LDADD = libcvs_lines.a -lomsee libcvs_lines.a

cvssearch_SOURCES = cvssearch.C
cvssearch_LDFLAGS = -static
cvssearch_LDADD = libcvs_lines.a -lomsee libcvs_lines.a

cvsminesearch_SOURCES =  cvsminesearch.C
cvsminesearch_LDFLAGS = -static
cvsminesearch_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

cvsfilesearch_SOURCES =   cvsfilesearch.C
cvsfilesearch_LDFLAGS = -static
cvsfilesearch_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

#cvsusagesearch_SOURCES =   cvsusagesearch.C
#cvsusagesearch_LDFLAGS = -static 
#cvsusagesearch_LDADD = libcvs_lines.a -lomsee libcvs_lines.a

#cvsclasses_SOURCES =   cvspatterns.C
#cvsclasses_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

#cvsfunctions_SOURCES =   cvspatterns.C
#cvsfunctions_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

#cvsusageindex_SOURCES =   cvsusageindex.C
#cvsusageindex_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a
#cvsusageindex_LDFLAGS = -static

cvsmineindex_SOURCES =   cvsmineindex.C
cvsmineindex_LDADD = libcvs_db.a libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a libcvs_db.a
cvsmineindex_LDFLAGS = -static

cvsfileindex_SOURCES =   cvsfileindex.C
cvsfileindex_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a
cvsfileindex_LDFLAGS = -static

#cvscomments_SOURCES = cvscomments.C
#cvscomments_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a
#cvscomments_LDFLAGS = -static

#cvssymbolcount_SOURCES = cvssymbolcount.C
#cvssymbolcount_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a
#cvssymbolcount_LDFLAGS = -static

#cvscountsymbols_SOURCES =   cvscountsymbols.C
#cvscountsymbols_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a
#cvscountsymbols_LDFLAGS = -static

#cvsminelocal_SOURCES =   cvsminelocal.C
#cvsminelocal_LDADD = libcvs_lines.a -lomsee -ldb_cxx libcvs_lines.a
#cvsminelocal_LDFLAGS = -static

#cvsmineglobal_SOURCES =   cvsmineglobal.C
#cvsmineglobal_LDADD = libcvs_lines.a -lomsee -ldb_cxx libcvs_lines.a


#cvseval_SOURCES =   cvseval.C

#cvscomments_SOURCES = cvscomments.C
#cvscomments_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

#cvssymbolcount_SOURCES = cvssymbolcount.C
#cvssymbolcount_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

#cvscountsymbols_SOURCES =   cvscountsymbols.C
#cvscountsymbols_LDADD = libcvs_lines.a -ldb_cxx -lomsee libcvs_lines.a

#cvsminelocal_SOURCES =   cvsminelocal.C
#cvsminelocal_LDADD = libcvs_lines.a -lomsee -ldb_cxx libcvs_lines.a

#cvsmineglobal_SOURCES =   cvsmineglobal.C
#cvsmineglobal_LDADD = libcvs_lines.a -lomsee -ldb_cxx libcvs_lines.a


#cvseval_SOURCES =   cvseval.C
#cvseval_LDADD = libcvs_lines.a -lomsee -ldb_cxx libcvs_lines.a


