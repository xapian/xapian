noinst_LIBRARIES = libcvs_db.a
libcvs_db_a_SOURCES = cvs_comment_db.C     cvs_db.C       cvs_file_revision_db.C  cvs_line_db.C \
 cvs_comment_db.h     cvs_db.h       cvs_file_revision_db.h  cvs_line_db.h \
 cvs_comment_id_db.C  cvs_db_file.C  cvs_filename_db.C       cvs_revision_db.C \
 cvs_comment_id_db.h  cvs_db_file.h  cvs_filename_db.h       cvs_revision_db.h \
 cvs_comment_id2_db.h cvs_comment_id2_db.C \
 cvs_file_id_db.h cvs_file_id_db.C \
 cvs_revision_less.h \
 cvs_diff_db.C cvs_diff_db.h

bin_PROGRAMS = cvsindex cvssearch cvsquery cvsmineindex cvsminesearch cvsfileindex cvsfilesearch \
	# cvssearch cvsusagesearch cvscomments \
	# cvssymbolcount cvscountsymbols cvsminelocal cvsquery #cvsmineglobal#cvseval

cvsquery_SOURCES = cvsquery.C
cvsquery_LDADD = libcvs_db.a -ldb_cxx libcvs_db.a

cvsindex_SOURCES = util.h util.C cvsindex.C
cvsindex_LDFLAGS = -static
cvsindex_LDADD = -lomsee

cvssearch_SOURCES = util.h util.C cvssearch.C
cvssearch_LDFLAGS = -static
cvssearch_LDADD = -lomsee

cvsminesearch_SOURCES = util.h util.C cvsminesearch.C
cvsminesearch_LDFLAGS = -static
cvsminesearch_LDADD = -ldb_cxx -lomsee

cvsfilesearch_SOURCES = util.h util.C cvsfilesearch.C
cvsfilesearch_LDFLAGS = -static
cvsfilesearch_LDADD = -ldb_cxx -lomsee

#cvsusagesearch_SOURCES = util.h util.C cvsusagesearch.C
#cvsusagesearch_LDFLAGS = -static 
#cvsusagesearch_LDADD = -lomsee

#cvsclasses_SOURCES = util.h util.C cvspatterns.C
#cvsclasses_LDADD = -ldb_cxx -lomsee

#cvsfunctions_SOURCES = util.h util.C cvspatterns.C
#cvsfunctions_LDADD = -ldb_cxx -lomsee

#cvsusageindex_SOURCES = util.h util.C cvsusageindex.C
#cvsusageindex_LDADD = -ldb_cxx -lomsee
#cvsusageindex_LDFLAGS = -static

cvsmineindex_SOURCES = util.h util.C cvsmineindex.C
cvsmineindex_LDADD = -ldb_cxx -lomsee
cvsmineindex_LDFLAGS = -static

cvsfileindex_SOURCES = util.h util.C cvsfileindex.C
cvsfileindex_LDADD = -ldb_cxx -lomsee
cvsfileindex_LDFLAGS = -static

#cvscomments_SOURCES = cvscomments.C
#cvscomments_LDADD = -ldb_cxx -lomsee
#cvscomments_LDFLAGS = -static

#cvssymbolcount_SOURCES = cvssymbolcount.C
#cvssymbolcount_LDADD = -ldb_cxx -lomsee
#cvssymbolcount_LDFLAGS = -static

#cvscountsymbols_SOURCES = util.h util.C cvscountsymbols.C
#cvscountsymbols_LDADD = -ldb_cxx -lomsee
#cvscountsymbols_LDFLAGS = -static

#cvsminelocal_SOURCES = util.h util.C cvsminelocal.C
#cvsminelocal_LDADD = -lomsee -ldb_cxx
#cvsminelocal_LDFLAGS = -static

#cvsmineglobal_SOURCES = util.h util.C cvsmineglobal.C
#cvsmineglobal_LDADD = -lomsee -ldb_cxx


#cvseval_SOURCES = util.h util.C cvseval.C

#cvscomments_SOURCES = cvscomments.C
#cvscomments_LDADD = -ldb_cxx -lomsee

#cvssymbolcount_SOURCES = cvssymbolcount.C
#cvssymbolcount_LDADD = -ldb_cxx -lomsee

#cvscountsymbols_SOURCES = util.h util.C cvscountsymbols.C
#cvscountsymbols_LDADD = -ldb_cxx -lomsee

#cvsminelocal_SOURCES = util.h util.C cvsminelocal.C
#cvsminelocal_LDADD = -lomsee -ldb_cxx

#cvsmineglobal_SOURCES = util.h util.C cvsmineglobal.C
#cvsmineglobal_LDADD = -lomsee -ldb_cxx


#cvseval_SOURCES = util.h util.C cvseval.C
#cvseval_LDADD = -lomsee -ldb_cxx


